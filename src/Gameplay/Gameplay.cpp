/**
 * Copyright (C) 2016 Martin Ubl <http://kennny.cz>
 *
 * This file is part of BubbleWorld MMORPG engine
 *
 * BubbleWorld is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BubbleWorld is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BubbleWorld. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "General.h"
#include "Gameplay.h"
#include "NetworkManager.h"
#include "Application.h"
#include "Map.h"
#include "StorageManager.h"
#include "MapStorage.h"
#include "Log.h"
#include "Creature.h"
#include "Gameobject.h"
#include "Player.h"
#include "Drawing.h"
#include "Colors.h"

#include "WorldObject.h"

Gameplay::Gameplay()
{
    m_hoverObject = nullptr;
}

void Gameplay::ConnectToServer()
{
    // TODO: real addresses taken from some kind of config

    sNetwork->Connect("127.0.0.1", 7874);
}

void Gameplay::Login(const char* username, const char* password)
{
    SmartPacket pkt(CP_LOGIN_REQUEST);
    pkt.WriteString(username);
    pkt.WriteString(password);
    pkt.WriteUInt32(APP_VERSION);
    sNetwork->SendPacket(pkt);
}

void Gameplay::RequestCharacterList()
{
    SmartPacket pkt(CP_CHARACTER_LIST_REQUEST);
    sNetwork->SendPacket(pkt);
}

void Gameplay::EnterWorld(uint32_t guid)
{
    m_playerGuid = guid;

    SmartPacket pkt(CP_ENTER_WORLD);
    pkt.WriteUInt32(guid);
    sNetwork->SendPacket(pkt);

    sApplication->SetStageType(STAGE_CONNECTING);
}

void Gameplay::Update()
{
    // update map only when in game stage
    if (sApplication->GetStageType() == STAGE_GAME && m_currentMap)
        m_currentMap->Update();
}

void Gameplay::MovementKeyEvent(MoveDirectionElement dir, bool press)
{
    // if player is created
    if (m_player)
    {
        // start movement on press
        if (press)
        {
            if (!m_player->IsMovingInDirection(dir))
            {
                SmartPacket pkt(CP_MOVE_START_DIRECTION);
                pkt.WriteUInt8(dir);
                sNetwork->SendPacket(pkt);

                m_player->StartMovementInDirection(dir);
            }
        }
        else // stop movement on release
        {
            if (m_player->IsMovingInDirection(dir))
            {
                SmartPacket pkt(CP_MOVE_STOP_DIRECTION);
                pkt.WriteUInt8(dir);
                pkt.WriteFloat(m_player->GetPositionX());
                pkt.WriteFloat(m_player->GetPositionY());
                sNetwork->SendPacket(pkt);

                m_player->StopMovementInDirection(dir);
            }
        }
    }
}

void Gameplay::CreatePlayer(uint32_t mapId, float posX, float posY)
{
    // create local player instance
    m_player = new Player();

    m_player->InitializeObject(m_playerGuid);
    m_player->SetMapId(mapId);
    m_player->SetPosition(posX, posY);

    LoadMap(mapId);
}

void Gameplay::LoadMap(uint32_t mapId)
{
    // get map from database
    MapDatabaseRecord* mrec = sMapStorage->GetMapRecord(mapId);
    if (!mrec)
    {
        // if not present, request it
        SendRequestMapMetadata(mapId);
        return;
    }

    // load map from file
    FILE* f;
    std::string path = DATA_DIR + mrec->filename;
    f = fopen(path.c_str(), "rb");
    if (!f)
    {
        // if file not found, request it
        SendRequestMapMetadata(mapId);
        return;
    }
    fclose(f);

    // create map instance
    m_currentMap = new Map();
    m_currentMap->SetId(mapId);

    // and verify checksum
    SendRequestMapMetadataChecksumVerify(mapId, mrec->headerChecksum.c_str());
}

void Gameplay::CreateMapUsing(MapHeader &mh)
{
    // create new map using header data
    m_currentMap = new Map();
    m_currentMap->InitEmpty(mh);
    m_currentMap->SaveToFile();

    // if the player is already created, add him to map
    if (m_player)
        m_currentMap->AddWorldObject(m_player);
}

void Gameplay::SignalMapLoaded(uint32_t mapId)
{
    // retrieve map record
    MapDatabaseRecord* mrec = sMapStorage->GetMapRecord(mapId);
    // the map record should be there; if not, it's fatal error
    if (!mrec)
    {
        sLog->Error("Could not load map ID %u, although it should be available!", mapId);
        return;
    }

    // load contents
    m_currentMap->LoadFromFile();

    // add player if not already there (AddWorldObject method checks that)
    if (m_player)
        m_currentMap->AddWorldObject(m_player);

    // store current location
    m_currentChunkX = m_currentMap->GetChunkIndexX((uint32_t)m_player->GetPositionX());
    m_currentChunkY = m_currentMap->GetChunkIndexY((uint32_t)m_player->GetPositionY());
    // request sorroundings
    RequestSorroundingChunks(true);

    // move stage to game
    sNetwork->SetConnectionState(CONNECTION_STATE_INGAME);
    sApplication->SetStageType(STAGE_GAME);

    // signal server about our arrival
    SmartPacket pkt(CP_WORLD_ENTER_COMPLETE);
    sNetwork->SendPacket(pkt);
}

void Gameplay::SignalNameQueryResolved(uint64_t guid, const wchar_t* name)
{
    if (!name)
        return;

    // cache name
    m_cachedNames[guid] = name;

    // let the object know about his name retrieval, if available
    if (m_currentMap)
    {
        if (WorldObject* obj = m_currentMap->GetWorldObject(guid))
            obj->SetName(name);
        else if (m_player && m_player->GetGUID() == guid)
            m_player->SetName(name);
    }
}

void Gameplay::SendRequestMapMetadata(uint32_t mapId)
{
    SmartPacket pkt(CP_GET_MAP_METADATA);
    pkt.WriteUInt32(mapId);
    sNetwork->SendPacket(pkt);
}

void Gameplay::SendRequestMapMetadataChecksumVerify(uint32_t mapId, const char* checksum)
{
    SmartPacket pkt(CP_MAP_METADATA_VERIFY_CHECKSUM);
    pkt.WriteUInt32(mapId);
    pkt.WriteString(checksum);
    sNetwork->SendPacket(pkt);
}

void Gameplay::SendRequestMapChunk(uint32_t mapId, uint32_t startX, uint32_t startY)
{
    SmartPacket pkt(CP_GET_MAP_CHUNK);
    pkt.WriteUInt32(mapId);
    pkt.WriteUInt32(startX);
    pkt.WriteUInt32(startY);
    sNetwork->SendPacket(pkt);
}

void Gameplay::SendRequestMapChunkChecksumVerify(uint32_t mapId, uint32_t startX, uint32_t startY, const char* checksum)
{
    // TODO: request chunk checksum just once per session!!! The chunk cannot change while the server is running!
    //       the Gameplay class expects SignalChunkLoaded callback call, so when the checksum had already been
    //       verified, just call this method and everything should work fine

    SmartPacket pkt(CP_MAP_CHUNK_VERIFY_CHECKSUM);
    pkt.WriteUInt32(mapId);
    pkt.WriteUInt32(startX);
    pkt.WriteUInt32(startY);
    pkt.WriteString(checksum);
    sNetwork->SendPacket(pkt);
}

void Gameplay::SendNameQuery(uint64_t guid)
{
    // if we already have that name resolved, fire it back
    if (m_cachedNames.find(guid) != m_cachedNames.end())
    {
        SignalNameQueryResolved(guid, m_cachedNames[guid].c_str());
        return;
    }

    // if we already sent request, cancel
    if (m_nameQuerySent.find(guid) != m_nameQuerySent.end() && m_nameQuerySent[guid] != 0)
        return;

    SmartPacket pkt(CP_NAME_QUERY);
    pkt.WriteInt64(guid);
    sNetwork->SendPacket(pkt);

    m_nameQuerySent[guid] = time(nullptr);
}

void Gameplay::SendChat(TalkType type, const wchar_t* str)
{
    std::string encoded = WStringToUTF8(str);

    SmartPacket pkt(CP_CHAT_MESSAGE);
    pkt.WriteUInt8(type);
    pkt.WriteString(encoded.c_str());
    sNetwork->SendPacket(pkt);
}

void Gameplay::SendInteractionRequest(WorldObject* object)
{
    // TODO: real distance check, that considers object images
    if (!object || !m_player || object->GetPosition().GetDistance(m_player->GetPosition()) > 1.0f)
        return;

    SmartPacket pkt(CP_INTERACTION_REQUEST);
    pkt.WriteUInt64(object->GetGUID());
    sNetwork->SendPacket(pkt);
}

void Gameplay::RequestSorroundingChunks(bool force)
{
    // retrieve current chunks
    uint32_t cellX = m_currentMap->GetChunkIndexX((uint32_t)m_player->GetPositionX());
    uint32_t cellY = m_currentMap->GetChunkIndexY((uint32_t)m_player->GetPositionY());

    uint32_t beginX, beginY, endX, endY, iX, iY, startX, startY;
    uint32_t beginX_old, beginY_old, endX_old, endY_old;

    // retrieve new sorroundings
    m_currentMap->GetCellSorroundingLimits(cellX, cellY, beginX, beginY, endX, endY);

    // retrieve old sorroundings (if not forced to not do so)
    if (!force)
        m_currentMap->GetCellSorroundingLimits(m_currentChunkX, m_currentChunkY, beginX_old, beginY_old, endX_old, endY_old);

    for (iX = beginX; iX <= endX; iX++)
    {
        startX = m_currentMap->GetChunkStartX(iX);

        for (iY = beginY; iY <= endY; iY++)
        {
            startY = m_currentMap->GetChunkStartY(iY);

            // if forced, or if we are moving to new chunks, request them
            if (force || iX > endX_old || iX < beginX_old || iY > endY_old || iY < beginY_old)
            {
                m_chunkLoadQueue.push_back(ChunkLoadQueueRecord(startX, startY));

                if (MapChunkDatabaseRecord* mrec = sMapStorage->GetMapChunkRecord(m_currentMap->GetId(), startX, startY))
                    SendRequestMapChunkChecksumVerify(m_currentMap->GetId(), startX, startY, mrec->checksum.c_str());
                else
                    SendRequestMapChunk(m_currentMap->GetId(), startX, startY);
            }
        }
    }
}

void Gameplay::SignalChunkLoaded(uint32_t startX, uint32_t startY)
{
    sLog->Info("Chunk [%u ; %u] loaded!", startX, startY);

    // erase chunk from loadlist
    for (std::list<ChunkLoadQueueRecord>::iterator itr = m_chunkLoadQueue.begin(); itr != m_chunkLoadQueue.end(); ++itr)
    {
        if ((*itr).startX == startX && (*itr).startY == startY)
        {
            m_chunkLoadQueue.erase(itr);
            break;
        }
    }

    // redraw, since there's new content
    sDrawing->SetCanvasRedrawFlag();

    if (m_chunkLoadQueue.empty())
    {
        // All chunks loaded

        // TODO: allow player movement here, or something like that
    }
}

void Gameplay::ClearCharacterList()
{
    for (std::list<CharacterListRecord*>::iterator itr = m_characterList.begin(); itr != m_characterList.end(); ++itr)
        delete *itr;

    m_characterList.clear();
}

void Gameplay::AddCharacterToList(CharacterListRecord* rec)
{
    m_characterList.push_back(rec);
}

std::list<CharacterListRecord*> const& Gameplay::GetCharacterList()
{
    return m_characterList;
}

Player* Gameplay::GetPlayer()
{
    return m_player;
}

Map* Gameplay::GetMap()
{
    return m_currentMap;
}

WorldObject* Gameplay::CreateForeignObject(uint64_t guid)
{
    // extract highguid
    uint32_t highguid = EXTRACT_GUIDHIGH(guid);
    // retrieve object type to create
    ObjectType type = _getTypeFromHighGUID(highguid);

    WorldObject* obj = nullptr;

    switch (type)
    {
        case OTYPE_PLAYER:
            obj = new Player();
            break;
        case OTYPE_CREATURE:
            obj = new Creature();
            break;
        case OTYPE_GAMEOBJECT:
            obj = new Gameobject();
            break;
        default:
            sLog->Error("Could not create object type %u (guidlow: %u)", type, EXTRACT_GUIDLOW(guid));
            break;
    }

    obj->InitializeObject(guid);

    return obj;
}

WorldObject* Gameplay::GetForeignObject(uint64_t guid)
{
    if (!m_currentMap)
        return nullptr;

    return m_currentMap->GetWorldObject(guid);
}

void Gameplay::AddChatMessage(TalkType type, const wchar_t* author, const wchar_t* message)
{
    std::wstring toPrint = L"";

    // server messages behaves differently
    if (type == TALK_SERVER_MESSAGE)
    {
        // prepare base
        std::wstring printmsg = std::wstring(L"[Server]: ") + message;
        // render wrapped message
        SDL_Surface* msgsurf = sDrawing->RenderFontWrappedUnicode(FONT_CHAT, printmsg.c_str(), CHAT_MSG_FRAME_WIDTH, BWCOLOR_CHAT_SERVERMSG);

        // store it for drawing
        m_chatMessages.push_back({ SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), msgsurf), time(nullptr) });

        // cleanup
        SDL_FreeSurface(msgsurf);
    }
    else
    {
        // prepare name and message surfaces
        SDL_Surface* namesurf = sDrawing->RenderFontUnicode(FONT_CHAT, author, BWCOLOR_CHAT_PLAYERNAME);
        std::wstring printmsg = std::wstring(L": ") + message;
        SDL_Surface* msgsurf = sDrawing->RenderFontWrappedUnicode(FONT_CHAT, printmsg.c_str(), CHAT_MSG_FRAME_WIDTH - namesurf->w, BWCOLOR_CHAT_TEXT);

        // create one bigger surface to contain both
        SDL_Surface* dstsurf = SDL_CreateRGBSurface(0, CHAT_MSG_FRAME_WIDTH, num_max(msgsurf->h, namesurf->h), 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

        // prepare name portion
        SDL_Rect dstrect;
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = namesurf->w;
        dstrect.h = namesurf->h;
        // draw name
        SDL_BlitSurface(namesurf, nullptr, dstsurf, &dstrect);

        // prepare message portion
        dstrect.x = namesurf->w;
        dstrect.y = 0;
        dstrect.w = msgsurf->w;
        dstrect.h = msgsurf->h;
        // draw message
        SDL_BlitSurface(msgsurf, nullptr, dstsurf, &dstrect);

        // store it for drawing
        m_chatMessages.push_back({ SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), dstsurf), time(nullptr) });

        // cleanup
        SDL_FreeSurface(namesurf);
        SDL_FreeSurface(msgsurf);
        SDL_FreeSurface(dstsurf);
    }

    // if the chat history is full, remove oldest message
    if (m_chatMessages.size() > CHAT_MSG_MAX_COUNT)
        m_chatMessages.pop_front();
}

std::list<ChatMessageRecord> const& Gameplay::GetChatMessages()
{
    return m_chatMessages;
}

void Gameplay::SetHoverObject(WorldObject* obj)
{
    // do not set hover object, when UI has hover - the UI has higher priority since it's "above" world
    if (sDrawing->HasUIWidgetHover())
    {
        m_hoverObject = nullptr;
        sDrawing->SetMouseCursor(MOUSE_CURSOR_NORMAL);
        return;
    }

    m_hoverObject = obj;

    // if some object gained hover, change cursor
    if (m_hoverObject)
    {
        // "talkable" creatures
        if (m_hoverObject->GetType() == OTYPE_CREATURE && m_hoverObject->ToCreature()->CanTalkTo())
            sDrawing->SetMouseCursor(MOUSE_CURSOR_TALK);
        else
            sDrawing->SetMouseCursor(MOUSE_CURSOR_NORMAL);

        // TODO: cursors for player, gameobject, enemy creature, ..
    }
    else
        sDrawing->SetMouseCursor(MOUSE_CURSOR_NORMAL);
}

WorldObject* Gameplay::GetHoverObject()
{
    return m_hoverObject;
}

void Gameplay::CheckHoverObject()
{
    if (!m_currentMap)
        return;

    WorldObject* obj;
    SDL_Rect* viewRect;

    WorldObject* hoverObj = nullptr;

    ObjectVector const& objvector = m_currentMap->GetObjectVisibilityVector();
    for (uint32_t i = 0; i < objvector.size(); i++)
    {
        obj = objvector[i];

        if (!obj->IsInView() || obj == m_player)
            continue;

        viewRect = obj->GetViewRect();

        // if the object is under mouse cursor, set hover
        if (sApplication->GetMouseX() >= viewRect->x && sApplication->GetMouseX() <= viewRect->x + viewRect->w &&
            sApplication->GetMouseY() >= viewRect->y && sApplication->GetMouseY() <= viewRect->y + viewRect->h)
        {
            hoverObj = obj;
        }
    }

    SetHoverObject(hoverObj);
}

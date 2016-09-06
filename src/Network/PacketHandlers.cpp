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
#include "PacketHandlers.h"
#include "SmartPacket.h"
#include "Application.h"
#include "NetworkManager.h"
#include "Gameplay.h"
#include "ResourceStreamManager.h"
#include "ResourceManager.h"
#include "Log.h"
#include "Map.h"
#include "MapStorage.h"
#include "StorageManager.h"
#include "CRC32.h"
#include "WorldObject.h"
#include "Creature.h"
#include "Unit.h"
#include "Player.h"
#include "Drawing.h"
#include "ItemCacheStorage.h"

#include <sstream>
#include <iomanip>

void PacketHandlers::Handle_NULL(SmartPacket& packet)
{
    // NULL handler - this means we throw away whole packet
}

void PacketHandlers::Handle_ServerSide(SmartPacket& packet)
{
    // This should never happen - we should never receive client-to-server packet
}

void PacketHandlers::HandleLoginResponse(SmartPacket& packet)
{
    uint8_t authState = packet.ReadUInt8();

    switch (authState)
    {
        // everything OK, signal user and retrieve character list
        case AUTH_STATUS_OK:
            sNetwork->SetConnectionState(CONNECTION_STATE_LOBBY);
            sApplication->SignalGlobalEvent(GA_CONNECTION_FETCHING);
            break;
        // unknown user
        case AUTH_STATUS_UNKNOWN_USER:
            sNetwork->Disconnect();
            sApplication->SignalGlobalEvent(GA_CONNECTION_INVALID_USER);
            break;
        // invalid password
        case AUTH_STATUS_INVALID_PASSWORD:
            sNetwork->Disconnect();
            sApplication->SignalGlobalEvent(GA_CONNECTION_INVALID_PASSWORD);
            break;
        // probably incompatible version
        case AUTH_STATUS_INCOMPATIBLE_VERSION:
            sNetwork->Disconnect();
            sApplication->SignalGlobalEvent(GA_CONNECTION_INCOMPATIBLE_VERSION);
            break;
        // blocked or banned account/IP
        case AUTH_STATUS_BANNED:
            sNetwork->Disconnect();
            sApplication->SignalGlobalEvent(GA_CONNECTION_BANNED);
            break;
    }
}

void PacketHandlers::HandleCharacterList(SmartPacket& packet)
{
    CharacterListRecord* lrec;

    uint8_t count = packet.ReadUInt8();

    // clear existing records
    sGameplay->ClearCharacterList();

    // read character list and put it into our list
    for (size_t i = 0; i < count; i++)
    {
        lrec = new CharacterListRecord;

        lrec->guid = packet.ReadUInt32();
        lrec->name = UTF8ToWString(packet.ReadString());
        lrec->level = packet.ReadUInt16();

        sGameplay->AddCharacterToList(lrec);
    }

    // acknowledge stage handle to create UI, etc.
    sApplication->SignalGlobalEvent(GA_CHARACTER_LIST_ACQUIRED);
}

void PacketHandlers::HandleResourceSendStart(SmartPacket& packet)
{
    std::string filename = packet.ReadString();
    ResourceType type = (ResourceType)packet.ReadUInt8();
    uint32_t id = packet.ReadUInt32();

    // create resource stream and await further packets
    sResourceStreamManager->CreateResourceStream(filename.c_str(), type, id);
}

void PacketHandlers::HandleResourceSendFinished(SmartPacket& packet)
{
    ResourceType type = (ResourceType)packet.ReadUInt8();
    uint32_t id = packet.ReadUInt32();

    sResourceStreamManager->FinishResourceStream(type, id);

    // we retrieve checksum after finishing retrieving file, and send verification packet
    const char* checksum = sResourceManager->GetResourceFileChecksum(type, id);

    sResourceStreamManager->SendVerifyChecksumPacket(type, id, checksum);
}

void PacketHandlers::HandleResourceData(SmartPacket& packet)
{
    ResourceType type = (ResourceType)packet.ReadUInt8();
    uint32_t id = packet.ReadUInt32();
    uint16_t size = packet.ReadUInt16();
    uint8_t* data = new uint8_t[size];

    // read bytes belonging to resource stream
    for (size_t i = 0; i < size; i++)
        data[i] = packet.ReadUInt8();

    sResourceStreamManager->WriteToResourceStream(type, id, size, data);

    delete data;
}

void PacketHandlers::HandleResourceChecksumVerify(SmartPacket& packet)
{
    ResourceType type;
    uint32_t id;

    // count of failed checksums
    uint16_t failed_count = packet.ReadUInt16();

    for (int i = 0; i < failed_count; i++)
    {
        type = (ResourceType)packet.ReadUInt8();
        id = packet.ReadUInt32();

        sLog->Debug("Checksum verify for resource type %u, id %u failed", type, id);

        // re-request resource if checksum does not match
        sResourceManager->RequestResource(type, id);
    }
}

void PacketHandlers::HandleEnterWorldResult(SmartPacket& packet)
{
    uint8_t status = packet.ReadUInt8();

    if (status != ENTER_WORLD_OK)
    {
        // TODO: error message; this should not happen at all
        sApplication->SetStageType(STAGE_MENU);
        return;
    }

    // retrieve position
    uint32_t mapId = packet.ReadUInt32();
    float posX = packet.ReadFloat();
    float posY = packet.ReadFloat();

    // create local player object
    sGameplay->CreatePlayer(mapId, posX, posY);
}

void PacketHandlers::HandleCreateObject(SmartPacket& packet)
{
    uint64_t guid;
    uint32_t fieldCount;
    uint8_t count = packet.ReadUInt8();

    WorldObject* obj;

    // read objects from packet
    for (uint32_t i = 0; i < count; i++)
    {
        // GUID
        guid = packet.ReadUInt64();
        // count of updatefields
        fieldCount = packet.ReadUInt32();

        // allocate and retrieve fields
        uint32_t* tmpFields = new uint32_t[fieldCount];

        for (uint32_t j = 0; j < fieldCount; j++)
            tmpFields[j] = packet.ReadUInt32();

        // if the create block does not identify local player...
        if (guid != sGameplay->GetPlayer()->GetGUID())
        {
            // create object
            obj = sGameplay->CreateForeignObject(guid);
            // apply updatefields
            obj->ApplyValueSet((uint8_t*)tmpFields, fieldCount * sizeof(uint32_t));

            // read position
            float x = packet.ReadFloat();
            float y = packet.ReadFloat();
            // set position
            obj->SetPosition(x, y);

            // if it's creature or player
            if (obj->GetType() == OTYPE_CREATURE || obj->GetType() == OTYPE_PLAYER)
            {
                // read movement info
                uint8_t moveMask = packet.ReadUInt8();
                // apply movement info
                for (uint8_t i = 0; i <= 3; i++)
                    if (((1 << i) & moveMask) != 0)
                        obj->ToUnit()->StartMovementInDirection((MoveDirectionElement)(1 << i));
            }

            // add to map
            sGameplay->GetMap()->AddWorldObject(obj);
        }
        else
        {
            // dummy read
            packet.ReadFloat();
            packet.ReadFloat();
            packet.ReadUInt8();

            // apply updatefields
            sGameplay->GetPlayer()->ApplyValueSet((uint8_t*)tmpFields, fieldCount * sizeof(uint32_t));
        }

        sDrawing->SetCanvasRedrawFlag();

        delete tmpFields;
    }
}

void PacketHandlers::HandleUpdateObject(SmartPacket& packet)
{
    uint64_t guid = packet.ReadUInt64();
    // find object
    WorldObject* obj = sGameplay->GetForeignObject(guid);
    if (!obj)
        return;

    uint32_t field, value;

    uint8_t count = packet.ReadUInt8();

    // update every field server sent
    for (uint8_t pos = 0; pos < count; pos++)
    {
        field = packet.ReadUInt32();
        value = packet.ReadUInt32();

        obj->SetUInt32Value(field, value);
    }

    // most likely something happened that would need redraw
    sDrawing->SetCanvasRedrawFlag();
}

void PacketHandlers::HandleDestroyObject(SmartPacket& packet)
{
    uint8_t count = packet.ReadUInt8();
    uint64_t guid;

    WorldObject* obj;
    // map has to to exist
    Map* map = sGameplay->GetMap();
    if (!map)
        return;

    // read objects to be destroyed
    for (uint8_t i = 0; i < count; i++)
    {
        // read guid and destroy it from map
        guid = packet.ReadUInt64();
        obj = sGameplay->GetForeignObject(guid);

        // remove from map
        map->RemoveWorldObject(guid);

        // cleanup
        if (obj)
            delete obj;
    }

    sDrawing->SetCanvasRedrawFlag();
}

void PacketHandlers::HandleMapMetadata(SmartPacket& packet)
{
    std::string name, filename;

    // create an nullify map header structure
    MapHeader mh;
    memset(&mh, 0, sizeof(MapHeader));

    // read status
    uint8_t status = packet.ReadUInt8();
    // has to be OK, otherwise we can't load map
    if (status != GENERIC_STATUS_OK)
        return;

    // set map header magic
    mh.mapVersionMagic = MAP_VERSION_MAGIC;
    // read basic info
    mh.mapId = packet.ReadUInt32();
    mh.sizeX = packet.ReadUInt32();
    mh.sizeY = packet.ReadUInt32();

    name = packet.ReadString();
    memset(mh.name, 0, MAP_NAME_LENGTH);
    for (int i = 0; i < name.length() && i < MAP_NAME_LENGTH; i++)
        mh.name[i] = name.at(i);

    mh.entryX = packet.ReadUInt32();
    mh.entryY = packet.ReadUInt32();
    mh.defaultFieldType = packet.ReadUInt16();
    mh.defaultFieldTexture = packet.ReadUInt32();
    mh.defaultFieldFlags = packet.ReadUInt32();

    filename = packet.ReadString();

    // get header CRC32 checksum
    uint32_t crc = CRC32_Bytes((uint8_t*)&mh, sizeof(MapHeader));

    std::string checksum = GetCRC32String(crc);

    // insert to local storage
    sMapStorage->InsertMapRecord(mh.mapId, filename.c_str(), checksum.c_str(), (uint32_t)time(nullptr));

    // create map in gameplay class
    sGameplay->CreateMapUsing(mh);

    // verify checksum
    sGameplay->SendRequestMapMetadataChecksumVerify(mh.mapId, checksum.c_str());
}

void PacketHandlers::HandleMapChunk(SmartPacket& packet)
{
    uint8_t status;
    uint32_t mapId, startX, startY, sizeX, sizeY;

    status = packet.ReadUInt8();
    // status has to be OK, otherwise chunk does not exist and we cannot load it
    if (status != GENERIC_STATUS_OK)
    {
        sLog->Error("Could not load requested map chunk");
        // TODO: some intelligent behaviour
        return;
    }

    // load metadata
    mapId = packet.ReadUInt32();
    startX = packet.ReadUInt32();
    startY = packet.ReadUInt32();
    sizeX = packet.ReadUInt32();
    sizeY = packet.ReadUInt32();

    Map* map = sGameplay->GetMap();
    MapField mf;

    uint32_t crc = 0;

    // read fields
    for (uint32_t i = 0; i < sizeX; i++)
    {
        for (uint32_t j = 0; j < sizeY; j++)
        {
            // nullify structure (due to padding, it also counts to CRC)
            memset(&mf, 0, sizeof(MapField));
            mf.type = packet.ReadUInt16();
            mf.texture = packet.ReadUInt32();
            mf.flags = packet.ReadUInt32();

            // calculate CRC32
            crc = CRC32_Bytes_Continuous((uint8_t*)&mf, sizeof(MapField), crc);

            // store field
            map->SetFieldContents(startX + i, startY + j, mf.type, mf.texture, mf.flags);
        }
    }

    // finalize CRC calculation
    crc = CRC32_Bytes_ContinuousFinalize(crc);

    std::string checksum = GetCRC32String(crc);

    // store to local file storage
    sMapStorage->InsertMapChunkRecord(mapId, startX, startY, sizeX, sizeY, checksum.c_str(), (uint32_t)time(nullptr));

    // save map to file to store changes
    map->SaveToFile();

    // send checksum verify packet
    sGameplay->SendRequestMapChunkChecksumVerify(mapId, startX, startY, checksum.c_str());
}

void PacketHandlers::HandleMapMetaChecksumVerify(SmartPacket& packet)
{
    uint8_t status = packet.ReadUInt8();
    uint32_t mapId = packet.ReadUInt32();

    // if map meta checksum OK, signal map load
    if (status == GENERIC_STATUS_OK)
        sGameplay->SignalMapLoaded(mapId);
    else // otherwise re-request map metadata
        sGameplay->SendRequestMapMetadata(mapId);
}

void PacketHandlers::HandleMapChunkChecksumVerify(SmartPacket& packet)
{
    uint8_t status = packet.ReadUInt8();
    uint32_t mapId = packet.ReadUInt32();
    uint32_t startX = packet.ReadUInt32();
    uint32_t startY = packet.ReadUInt32();

    // if chunk checksum OK, signal chunk load
    if (status == GENERIC_STATUS_OK)
        sGameplay->SignalChunkLoaded(startX, startY);
    else // otherwise re-request chunk
        sGameplay->SendRequestMapChunk(mapId, startX, startY);
}

void PacketHandlers::HandleImageMetadata(SmartPacket& packet)
{
    uint32_t id, sizeX, sizeY, baseCenterX, baseCenterY, collisionX1, collisionY1, collisionX2, collisionY2, animCount;
    uint32_t animId, frameBegin, frameEnd, frameDelay;

    uint8_t status = packet.ReadUInt8();
    // status has to be OK, otherwise we cannot load it
    if (status != GENERIC_STATUS_OK)
    {
        sLog->Error("Could not load requested image metadata");
        // TODO: some intelligent behaviour
        return;
    }

    uint32_t crc = 0;

    // read meta
    id = packet.ReadUInt32();
    sizeX = packet.ReadUInt32();
    sizeY = packet.ReadUInt32();
    baseCenterX = packet.ReadUInt32();
    baseCenterY = packet.ReadUInt32();
    collisionX1 = packet.ReadUInt32();
    collisionY1 = packet.ReadUInt32();
    collisionX2 = packet.ReadUInt32();
    collisionY2 = packet.ReadUInt32();

    // perform checksum on metadata
    crc = CRC32_Bytes_Continuous((uint8_t*)&id, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&sizeX, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&sizeY, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&baseCenterX, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&baseCenterY, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&collisionX1, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&collisionY1, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&collisionX2, sizeof(uint32_t), crc);
    crc = CRC32_Bytes_Continuous((uint8_t*)&collisionY2, sizeof(uint32_t), crc);

    // wipe existing metadata
    sImageStorage->WipeImageMetadata(id);

    // animation count
    animCount = packet.ReadUInt32();

    // read anim metadata
    for (uint32_t i = 0; i < animCount; i++)
    {
        // read data
        animId = packet.ReadUInt32();
        frameBegin = packet.ReadUInt32();
        frameEnd = packet.ReadUInt32();
        frameDelay = packet.ReadUInt32();

        // perform checksum
        crc = CRC32_Bytes_Continuous((uint8_t*)&animId, sizeof(uint32_t), crc);
        crc = CRC32_Bytes_Continuous((uint8_t*)&frameBegin, sizeof(uint32_t), crc);
        crc = CRC32_Bytes_Continuous((uint8_t*)&frameEnd, sizeof(uint32_t), crc);
        crc = CRC32_Bytes_Continuous((uint8_t*)&frameDelay, sizeof(uint32_t), crc);

        // insert animation record to local file storage
        sImageStorage->InsertImageAnimationRecord(id, animId, frameBegin, frameEnd, frameDelay, (uint32_t)time(nullptr));
    }

    // finalize CRC32 calculation
    crc = CRC32_Bytes_ContinuousFinalize(crc);

    std::string checksum = GetCRC32String(crc);

    // insert metadata parent record to local file database
    sImageStorage->InsertImageMetadataRecord(id, sizeX, sizeY, baseCenterX, baseCenterY, collisionX1, collisionY1, collisionX2, collisionY2, checksum.c_str(), (uint32_t)time(nullptr));

    // send metadata checksum verify packet
    sResourceStreamManager->SendVerifyMetadataChecksumPacket(RSTYPE_IMAGE, id, checksum.c_str());
}

void PacketHandlers::HandleImageMetaChecksumVerify(SmartPacket& packet)
{
    uint8_t status = packet.ReadUInt8();
    uint32_t id = packet.ReadUInt32();

    // if OK, signal resource metadata retrieval to allow loading
    if (status == GENERIC_STATUS_OK)
        sResourceManager->SignalResourceMetadataRetrieved(RSTYPE_IMAGE, id);
    else // otherwise request again
        sResourceManager->RequestResourceMetadata(RSTYPE_IMAGE, id);
}

void PacketHandlers::HandleNameQueryResponse(SmartPacket& packet)
{
    uint64_t guid = packet.ReadUInt64();
    std::string name = packet.ReadString();

    // just pass the reply to gameplay class
    sGameplay->SignalNameQueryResolved(guid, (name.length() == 0) ? L"???" : UTF8ToWString(name).c_str());
}

void PacketHandlers::HandleMoveStartDir(SmartPacket& packet)
{
    uint64_t guid = packet.ReadUInt64();
    uint8_t dir = packet.ReadUInt8();

    // ignore updates about self, we handle it our way
    if (sGameplay->GetPlayer()->GetGUID() == guid)
        return;

    // retrieve foreign object
    WorldObject* target = sGameplay->GetForeignObject(guid);
    if (!target || (target->GetType() != OTYPE_PLAYER && target->GetType() != OTYPE_CREATURE))
        return;

    // pass the event
    target->ToUnit()->StartMovementInDirection((MoveDirectionElement)dir);
}

void PacketHandlers::HandleMoveStopDir(SmartPacket& packet)
{
    uint64_t guid = packet.ReadUInt64();
    uint8_t dir = packet.ReadUInt8();
    float x = packet.ReadFloat();
    float y = packet.ReadFloat();

    // ignore updates about self, we handle it our way
    if (sGameplay->GetPlayer()->GetGUID() == guid)
        return;

    // retrieve foreign object
    WorldObject* target = sGameplay->GetForeignObject(guid);
    if (!target || (target->GetType() != OTYPE_PLAYER && target->GetType() != OTYPE_CREATURE))
        return;

    // update position and stop movement in that way
    target->SetPosition(x, y);
    target->ToUnit()->StopMovementInDirection((MoveDirectionElement)dir);
}

void PacketHandlers::HandleMoveHeartbeat(SmartPacket& packet)
{
    uint64_t guid = packet.ReadUInt64();
    uint8_t moveMask = packet.ReadUInt8();
    float x = packet.ReadFloat();
    float y = packet.ReadFloat();

    // ignore updates about self, we handle it our way
    if (sGameplay->GetPlayer()->GetGUID() == guid)
        return;

    // retrieve foreign object
    WorldObject* target = sGameplay->GetForeignObject(guid);
    if (!target || (target->GetType() != OTYPE_PLAYER && target->GetType() != OTYPE_CREATURE))
        return;

    // update position
    target->SetPosition(x, y);

    // TODO: something with moveMask ?
}

void PacketHandlers::HandleChatMessage(SmartPacket& packet)
{
    uint8_t type = packet.ReadUInt8();
    uint64_t guid = packet.ReadUInt64();
    std::string msg = packet.ReadString();

    std::wstring wmsg = UTF8ToWString(msg);

    // server messages have its own behaviour
    if (type != TALK_SERVER_MESSAGE)
    {
        // retrieve talk unit
        WorldObject* talkunit = sGameplay->GetForeignObject(guid);
        if (talkunit && (talkunit->GetType() == OTYPE_CREATURE || talkunit->GetType() == OTYPE_PLAYER))
        {
            // talk and add to history
            talkunit->ToUnit()->Talk((TalkType)type, wmsg.c_str());
            sGameplay->AddChatMessage((TalkType)type, talkunit->GetName(), wmsg.c_str());
        }
    }
    else
    {
        sGameplay->AddChatMessage((TalkType)type, nullptr, wmsg.c_str());
    }
}

void PacketHandlers::HandleDialogueData(SmartPacket& packet)
{
    uint64_t sourceGuid = packet.ReadUInt64();
    uint8_t dialogueState = packet.ReadUInt8();

    // "wait" means the server will signal us when something new happens
    if (dialogueState == DIALOGUE_WAIT)
    {
        sGameplay->StartOrResetDialogue(sourceGuid, L"Probíhá rozhovor...");
    }
    // "decide" means we have to choose an alternative
    else if (dialogueState == DIALOGUE_DECIDE)
    {
        std::string headerText = packet.ReadString();
        std::wstring wmsg = UTF8ToWString(headerText);

        // this will start a new dialogue or reuse old one
        sGameplay->StartOrResetDialogue(sourceGuid, wmsg.c_str());

        uint32_t id;
        std::string decString;

        // read all received decisions
        uint8_t count = packet.ReadUInt8();
        for (uint8_t i = 0; i < count; i++)
        {
            id = packet.ReadUInt32();
            decString = packet.ReadString();
            wmsg = UTF8ToWString(decString);

            // and put them to dialogue
            sGameplay->AddDialogueDecision(id, wmsg.c_str());
        }
    }
}

void PacketHandlers::HandleDialogueClose(SmartPacket& packet)
{
    // just end dialogue

    sGameplay->EndDialogue();
}

void PacketHandlers::HandleInventory(SmartPacket& packet)
{
    uint32_t guid, id, stackcount;
    sGameplay->ClearInventoryRecords();

    for (uint32_t i = 0; i < CHARACTER_INVENTORY_SLOTS; i++)
    {
        guid = packet.ReadUInt32();
        if (!guid)
            continue;

        id = packet.ReadUInt32();
        stackcount = packet.ReadUInt32();

        sGameplay->SetInventorySlotContents(i, guid, id, stackcount);
    }
}

void PacketHandlers::HandleItemQueryResponse(SmartPacket& packet)
{
    uint8_t status = packet.ReadUInt8();
    if (status != GENERIC_STATUS_OK)
        return;

    uint32_t id = packet.ReadUInt32();
    uint32_t imageId = packet.ReadUInt32();
    std::wstring name = UTF8ToWString(packet.ReadString());
    std::wstring description = UTF8ToWString(packet.ReadString());
    uint32_t stackSize = packet.ReadUInt32();
    uint32_t rarity = packet.ReadUInt32();

    sItemCache->AddItemCacheEntry(id, imageId, name.c_str(), description.c_str(), stackSize, rarity, (uint32_t)time(nullptr));

    sGameplay->SignalItemCacheEntryLoaded(id);
}

void PacketHandlers::HandleItemOperationInfo(SmartPacket& packet)
{
    uint8_t operation = packet.ReadUInt8();
    uint32_t itemId = packet.ReadUInt32();
    uint32_t count = packet.ReadUInt32();

    sGameplay->ReportItemOperation(itemId, (ItemInventoryOperation)operation, count);
}

void PacketHandlers::HandleUpdateInventorySlot(SmartPacket& packet)
{
    uint32_t slot, guid, id, count;

    slot = packet.ReadUInt32();
    guid = packet.ReadUInt32();
    // if guid equals zero, that means the slot is empty at all and no related data is appended
    if (guid == 0)
        sGameplay->SetInventorySlotContents(slot, 0, 0, 0);
    else
    {
        id = packet.ReadUInt32();
        count = packet.ReadUInt32();

        sGameplay->SetInventorySlotContents(slot, guid, id, count);
    }
}

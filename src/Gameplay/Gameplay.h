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

#ifndef BW_GAMEPLAY_H
#define BW_GAMEPLAY_H

#include "Singleton.h"

class WorldObject;
class Map;
class Player;
class DialogueWidget;
struct MapHeader;
struct ItemCacheEntry;
enum MoveDirectionElement;

// the maximum distance for player/object interaction
#define INTERACTION_DISTANCE_ABSOLUTE 1.25f
// maximum number of slots the character could have in his inventory
#define CHARACTER_INVENTORY_SLOTS 100

/*
 * Structure for character list record
 */
struct CharacterListRecord
{
    // character guid (used in world enter packet)
    uint32_t guid;
    // character name
    std::wstring name;
    // character level
    uint16_t level;

    // TODO: more
};

/*
 * Queue record for loading map chunks
 */
struct ChunkLoadQueueRecord
{
    ChunkLoadQueueRecord(uint32_t sX, uint32_t sY) : startX(sX), startY(sY) {};

    // startX coordinate of chunk
    uint32_t startX;
    // startY coordinate of chunk
    uint32_t startY;
};

/*
 * Record of chat message (rendered)
 */
struct ChatMessageRecord
{
    // cached prerendered texture)
    SDL_Texture* texture;
    // chat message receive time
    time_t addedTimestamp;
};

/*
 * Structure containing information about item in inventory
 */
struct InventoryItem
{
    // item GUID
    uint32_t guid;
    // item ID
    uint32_t id;
    // count of items in this slot
    uint32_t stackCount;
};

/*
 * Structure containing information about delayed item operation to be reported in chat
 */
struct DelayedItemOperationInfoRecord
{
    DelayedItemOperationInfoRecord(uint32_t _itemId, uint32_t _count, ItemInventoryOperation _operation) : itemId(_itemId), count(_count), operation(_operation) {};

    // item ID
    uint32_t itemId;
    // count of items
    uint32_t count;
    // the operation to be reported
    ItemInventoryOperation operation;
};

/*
 * Singleton class for maintaining gameplay related mechanics
 */
class Gameplay
{
    friend class Singleton<Gameplay>;
    public:
        // connect to server using values from config
        void ConnectToServer();
        // sends login packet with specified username+password
        void Login(const char* username, const char* password);
        // send request character list packet
        void RequestCharacterList();
        // send world enter packet
        void EnterWorld(uint32_t guid);

        // updates entities and maps
        void Update();
        // when player presses key related to movement
        void MovementKeyEvent(MoveDirectionElement dir, bool press);
        // send packet for requesting map metadata
        void SendRequestMapMetadata(uint32_t mapId);
        // send packet for verifying map metadata checksum
        void SendRequestMapMetadataChecksumVerify(uint32_t mapId, const char* checksum);
        // send packet for requesting map chunk
        void SendRequestMapChunk(uint32_t mapId, uint32_t startX, uint32_t startY);
        // send packet for verifying map chunk checksum
        void SendRequestMapChunkChecksumVerify(uint32_t mapId, uint32_t startX, uint32_t startY, const char* checksum);
        // send name query packet for requesting name of object
        void SendNameQuery(uint64_t guid);
        // sends chat message
        void SendChat(TalkType type, const wchar_t* str);
        // sends interaction request
        void SendInteractionRequest(WorldObject* object);
        // sends inventory request
        void SendInventoryRequest();
        // sends request for swapping two slots in inventory
        void SendSwapInventorySlots(uint32_t src, uint32_t dst);
        // sends request for item deletion (in slot)
        void SendRemoveInventorySlot(uint32_t slot);
        // sends query for item
        void SendItemQuery(uint32_t id);

        // creates local player on map
        void CreatePlayer(uint32_t mapId, float posX, float posY);
        // loads map to be used for gameplay
        void LoadMap(uint32_t mapId);
        // creates new map (if not available before)
        void CreateMapUsing(MapHeader &mh);
        // signals gameplay class about map resolve event (also verified)
        void SignalMapLoaded(uint32_t mapId);
        // signals gameplay class about name resolve event
        void SignalNameQueryResolved(uint64_t guid, const wchar_t* name);

        // clears character list (lobby)
        void ClearCharacterList();
        // add character to list (lobby)
        void AddCharacterToList(CharacterListRecord* rec);
        // retrieves character list (lobby)
        std::list<CharacterListRecord*> const& GetCharacterList();

        // retrieves local player
        Player* GetPlayer();
        // retrieves current map
        Map* GetMap();

        // send request packets for chunks sorrounding current player; force parameter causes ignoring cached state
        void RequestSorroundingChunks(bool force = false);
        // signals gameplay class, that the chunk was successfully retrieved and loaded
        void SignalChunkLoaded(uint32_t startX, uint32_t startY);

        // creates object in world using only guid for initialization
        WorldObject* CreateForeignObject(uint64_t guid);
        // retrieves foreign object from current map
        WorldObject* GetForeignObject(uint64_t guid);

        // adds chat message to history
        void AddChatMessage(TalkType type, const wchar_t* author, const wchar_t* message);
        // retrieves chat message history
        std::list<ChatMessageRecord> const& GetChatMessages();

        // sets hover object (the mouse is over that object)
        void SetHoverObject(WorldObject* obj);
        // retrieves object, over which the mouse currently is
        WorldObject* GetHoverObject();
        // checks if the mouse changes hover object
        void CheckHoverObject();

        // starts or resets dialogue with NPC
        void StartOrResetDialogue(uint64_t sourceGuid, const wchar_t* headerText);
        // adds dialogue decision to current dialogue widget
        void AddDialogueDecision(uint32_t id, const wchar_t* text);
        // ends dialogue, clears widget
        void EndDialogue();
        // when dialogue button signals gameplay class about decision made (button clicked)
        void SignalDialogueDecision(uint32_t id);

        // retrieves item cache entry if present - if not, sends request
        ItemCacheEntry* GetItemCacheEntry(uint32_t id);
        // clears inventory records
        void ClearInventoryRecords();
        // sets inventory slot contents
        void SetInventorySlotContents(uint32_t slot, uint32_t guid, uint32_t id, uint32_t stackCount);
        // signals item cache entry was loaded
        void SignalItemCacheEntryLoaded(uint32_t id);
        // retrieves inventory slot contents
        InventoryItem* GetInventorySlot(uint32_t slot);
        // reports item operation
        void ReportItemOperation(uint32_t itemId, ItemInventoryOperation operation, uint32_t count, bool delayed = false);

    protected:
        // protected singleton constructor
        Gameplay();

        // checks delayed item operation list and reports newly loaded items
        void CheckDelayedItemOperationsFor(uint32_t itemId);

    private:
        // guid of current player
        uint32_t m_playerGuid;
        // character list used in lobby stage
        std::list<CharacterListRecord*> m_characterList;

        // currently loaded map
        Map* m_currentMap;
        // local player instance
        Player* m_player;
        // current player chunk X coordinate
        uint32_t m_currentChunkX;
        // current player chunk Y coordinate
        uint32_t m_currentChunkY;

        // queue of chunks that are currently being retrieved or loaded
        std::list<ChunkLoadQueueRecord> m_chunkLoadQueue;
        // set of item queries that has been sent
        std::set<uint32_t> m_itemQuerySent;
        // chat message history
        std::list<ChatMessageRecord> m_chatMessages;

        // map of guid+timestamp of name retrieval attempts
        std::unordered_map<uint64_t, time_t> m_nameQuerySent;
        // cached names of objects
        std::unordered_map<uint64_t, std::wstring> m_cachedNames;
        // current mouseover object
        WorldObject* m_hoverObject;
        // current dialogue widget
        DialogueWidget* m_dialogueWidget;
        // current dialogue source object GUID
        uint64_t m_dialogueSourceGUID;

        // character inventory
        InventoryItem* m_inventory[CHARACTER_INVENTORY_SLOTS];
        // list of delayed item operations
        std::list<DelayedItemOperationInfoRecord> m_delayedItemOperationInfo;
};

#define sGameplay Singleton<Gameplay>::getInstance()

#endif

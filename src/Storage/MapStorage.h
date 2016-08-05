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

#ifndef BW_MAPSTORAGE_H
#define BW_MAPSTORAGE_H

#include "MapEnums.h"
#include "FileStorage.h"

/*
 * Structure containing map database record data
 */
struct MapDatabaseRecord
{
    // map ID
    uint32_t id;
    // map filename
    std::string filename;
    // checksum of header
    std::string headerChecksum;
    // timestamp of record creation
    uint32_t addedTimestamp;
};

typedef std::unordered_map<uint32_t, MapDatabaseRecord> MapRecordMap;

/*
 * Structure containing map chunk database record data
 */
struct MapChunkDatabaseRecord
{
    // map ID
    uint32_t mapId;
    // starting X coordinate
    uint32_t startX;
    // starting Y coordinate
    uint32_t startY;
    // count of fields in X direction
    uint32_t sizeX;
    // count of fields in Y direction
    uint32_t sizeY;
    // content checksum
    std::string checksum;
    // timestamp of record creation
    uint32_t addedTimestamp;
};

typedef std::unordered_map<uint32_t, MapChunkDatabaseRecord> MapChunkRecordRow;
typedef std::unordered_map<uint32_t, MapChunkRecordRow> MapChunkRecordMap;
typedef std::unordered_map<uint32_t, MapChunkRecordMap> AllMapChunkRecordMap;

/*
 * Class for maintaining map file storage
 */
class MapStorage : public FileStorage
{
    public:
        MapStorage();
        ~MapStorage();

        void CreateTablesIfNotExist();
        void Load();

        // inserts new map record
        void InsertMapRecord(uint32_t id, const char* filename, const char* headerChecksumStr, uint32_t addedTimestamp);
        // retrieves map record
        MapDatabaseRecord* GetMapRecord(uint32_t id);
        // inserts new map chunk record
        void InsertMapChunkRecord(uint32_t mapId, uint32_t startX, uint32_t startY, uint32_t sizeX, uint32_t sizeY, const char* checksumStr, uint32_t addedTimestamp);
        // retrieves map chunk record
        MapChunkDatabaseRecord* GetMapChunkRecord(uint32_t mapId, uint32_t startX, uint32_t startY);

    protected:
        //

    private:
        // map of map records; key = mapId
        MapRecordMap m_mapRecords;
        // map of map chunk records; key1 = mapId, key2 = startX, key3 = startY
        AllMapChunkRecordMap m_chunkRecords;
};

#endif

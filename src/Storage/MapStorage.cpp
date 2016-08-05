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
#include "StorageManager.h"
#include "FileStorage.h"
#include "MapStorage.h"
#include "Log.h"

MapStorage::MapStorage() : FileStorage(SQLITE_DB_MAP)
{
    //
}

MapStorage::~MapStorage()
{
    //
}

void MapStorage::CreateTablesIfNotExist()
{
    DBExecute("CREATE TABLE IF NOT EXISTS map (id INTEGER, filename TEXT, header_checksum TEXT, added_time INTEGER);");
    DBExecute("CREATE TABLE IF NOT EXISTS map_chunk (map_id INTEGER, start_x INTEGER, start_y INTEGER, size_x INTEGER, size_y INTEGER, checksum TEXT, added_time INTEGER);");
}

void MapStorage::Load()
{
    m_mapRecords.clear();

    Query* qr = DBQuery("SELECT * FROM map");

    uint32_t count = 0;
    uint32_t id;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            m_mapRecords[id].id = id;
            m_mapRecords[id].filename = qr->getstr();
            m_mapRecords[id].headerChecksum = qr->getstr();
            m_mapRecords[id].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("MapStorage: Loaded %u cached maps", count);

    qr = DBQuery("SELECT * FROM map_chunk");

    count = 0;
    uint32_t startX, startY;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            startX = qr->getuval();
            startY = qr->getuval();
            m_chunkRecords[id][startX][startY].mapId = id;
            m_chunkRecords[id][startX][startY].startX = startX;
            m_chunkRecords[id][startX][startY].startY = startY;
            m_chunkRecords[id][startX][startY].sizeX = qr->getuval();
            m_chunkRecords[id][startX][startY].sizeY = qr->getuval();
            m_chunkRecords[id][startX][startY].checksum = qr->getstr();
            m_chunkRecords[id][startX][startY].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("MapStorage: Loaded %u cached map chunks", count);
}

void MapStorage::InsertMapRecord(uint32_t id, const char* filename, const char* headerChecksumStr, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM map WHERE id = %u;", id);
    DBExecute("INSERT INTO map (id, filename, header_checksum, added_time) VALUES (%u, '%s', '%s', %u);", id, filename, headerChecksumStr, addedTimestamp);

    m_mapRecords[id].id = id;
    m_mapRecords[id].filename = filename;
    m_mapRecords[id].headerChecksum = headerChecksumStr;
    m_mapRecords[id].addedTimestamp = addedTimestamp;
}

MapDatabaseRecord* MapStorage::GetMapRecord(uint32_t id)
{
    if (m_mapRecords.find(id) == m_mapRecords.end())
        return nullptr;

    return &m_mapRecords[id];
}

void MapStorage::InsertMapChunkRecord(uint32_t mapId, uint32_t startX, uint32_t startY, uint32_t sizeX, uint32_t sizeY, const char* checksumStr, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM map_chunk WHERE map_id = %u AND start_x = %u AND start_y = %u;", mapId, startX, startY);
    DBExecute("INSERT INTO map_chunk (map_id, start_x, start_y, size_x, size_y, checksum, added_time) VALUES (%u, %u, %u, %u, %u, '%s', %u);", mapId, startX, startY, sizeX, sizeY, checksumStr, addedTimestamp);

    m_chunkRecords[mapId][startX][startY].mapId = mapId;
    m_chunkRecords[mapId][startX][startY].startX = startX;
    m_chunkRecords[mapId][startX][startY].startY = startY;
    m_chunkRecords[mapId][startX][startY].sizeX = sizeX;
    m_chunkRecords[mapId][startX][startY].sizeY = sizeY;
    m_chunkRecords[mapId][startX][startY].checksum = checksumStr;
    m_chunkRecords[mapId][startX][startY].addedTimestamp = addedTimestamp;
}

MapChunkDatabaseRecord* MapStorage::GetMapChunkRecord(uint32_t mapId, uint32_t startX, uint32_t startY)
{
    if (m_chunkRecords.find(mapId) == m_chunkRecords.end())
        return nullptr;

    if (m_chunkRecords[mapId].find(startX) == m_chunkRecords[mapId].end())
        return nullptr;

    if (m_chunkRecords[mapId][startX].find(startY) == m_chunkRecords[mapId][startX].end())
        return nullptr;

    return &m_chunkRecords[mapId][startX][startY];
}

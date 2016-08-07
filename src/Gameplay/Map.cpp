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
#include "Map.h"
#include "Log.h"
#include "MapStorage.h"
#include "StorageManager.h"
#include "WorldObject.h"
#include "Gameplay.h"

Map::Map()
{
    m_header.mapId = 0;
    m_objectVisibilityVector.clear();
}

Map::~Map()
{
    //
}

void Map::InitEmpty(MapHeader &mh)
{
    // copy map header
    memcpy(&m_header, &mh, sizeof(MapHeader));
    // include version magic (to allow saving raw header to file without additional code)
    m_header.mapVersionMagic = MAP_VERSION_MAGIC;

    // initialize map fields with default values
    m_fields.resize(m_header.sizeX);
    for (uint32_t ix = 0; ix < m_header.sizeX; ix++)
    {
        m_fields[ix].clear();
        m_fields[ix].resize(m_header.sizeY);
        for (uint32_t iy = 0; iy < m_header.sizeY; iy++)
        {
            m_fields[ix][iy].type = m_header.defaultFieldType;
            m_fields[ix][iy].texture = m_header.defaultFieldTexture;
            m_fields[ix][iy].flags = m_header.defaultFieldFlags;
        }
    }
}

void Map::Update()
{
    // update all objects on map
    for (uint32_t i = 0; i < m_objectVisibilityVector.size(); i++)
        m_objectVisibilityVector[i]->Update();
}

void Map::SetId(uint32_t id)
{
    m_header.mapId = id;
}

uint32_t Map::GetId()
{
    return m_header.mapId;
}

void Map::SetFieldContents(uint32_t x, uint32_t y, uint16_t type, uint32_t texture, uint32_t flags)
{
    // secure range
    if (m_fields.size() <= x || m_fields[x].size() <= y)
    {
        sLog->Error("Attempt to set nonexistant field (X = %u, Y = %u)", x, y);
        return;
    }

    MapField* mf = &m_fields[x][y];
    memset(mf, 0, sizeof(MapField));
    mf->type = type;
    mf->texture = texture;
    mf->flags = flags;
}

MapField* Map::GetField(uint32_t x, uint32_t y)
{
    if (m_fields.size() <= x || m_fields[x].size() <= y)
        return nullptr;

    return &m_fields[x][y];
}

MapField const* Map::GetField_unsafe(uint32_t x, uint32_t y) const
{
    return &m_fields[x][y];
}

uint32_t Map::GetChunkIndexX(uint32_t startX)
{
    return startX / MAP_CHUNK_SIZE_X;
}

uint32_t Map::GetChunkIndexY(uint32_t startY)
{
    return startY / MAP_CHUNK_SIZE_Y;
}

uint32_t Map::GetChunkStartX(uint32_t indexX)
{
    return indexX * MAP_CHUNK_SIZE_X;
}

uint32_t Map::GetChunkStartY(uint32_t indexY)
{
    return indexY * MAP_CHUNK_SIZE_Y;
}

void Map::GetCellSorroundingLimits(uint32_t cellX, uint32_t cellY, uint32_t &beginX, uint32_t &beginY, uint32_t &endX, uint32_t &endY)
{
    beginX = cellX > MAP_SORROUNDING_CELLS_X ? cellX - MAP_SORROUNDING_CELLS_X : 0;
    beginY = cellY > MAP_SORROUNDING_CELLS_Y ? cellY - MAP_SORROUNDING_CELLS_Y : 0;

    uint32_t limitX = GetChunkIndexX((uint32_t) m_fields.size() - 1);
    uint32_t limitY = GetChunkIndexY((uint32_t) m_fields[0].size() - 1);

    endX = cellX + MAP_SORROUNDING_CELLS_X < limitX ? cellX + MAP_SORROUNDING_CELLS_X : limitX;
    endY = cellY + MAP_SORROUNDING_CELLS_Y < limitY ? cellY + MAP_SORROUNDING_CELLS_Y : limitY;
}

bool Map::LoadFromFile()
{
    MapDatabaseRecord* mrec = sMapStorage->GetMapRecord(m_header.mapId);
    if (!mrec)
    {
        sLog->Error("Attempt to load nonexistant map (ID %u)", m_header.mapId);
        return false;
    }

    std::string path = DATA_DIR + mrec->filename;

    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "rb");
    if (!f)
    {
        sLog->Error("Could not open file %s for reading", mrec->filename.c_str());
        return false;
    }

    fread(&m_header, sizeof(MapHeader), 1, f);

    m_fields.resize(m_header.sizeX);
    for (uint32_t x = 0; x < m_header.sizeX; x++)
    {
        m_fields[x].resize(m_header.sizeY);
        for (uint32_t y = 0; y < m_header.sizeY; y++)
        {
            fread(&m_fields[x][y], sizeof(MapField), 1, f);
        }
    }

    fclose(f);

    return true;
}

void Map::SaveToFile()
{
    MapDatabaseRecord* mrec = sMapStorage->GetMapRecord(m_header.mapId);
    if (!mrec)
    {
        sLog->Error("Attempt to save nonexistant map (ID %u)", m_header.mapId);
        return;
    }

    std::string path = DATA_DIR + mrec->filename;

    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "wb");
    if (!f)
    {
        sLog->Error("Could not open file %s for writing", mrec->filename.c_str());
        return;
    }

    fwrite(&m_header, sizeof(MapHeader), 1, f);

    for (uint32_t x = 0; x < m_header.sizeX; x++)
    {
        for (uint32_t y = 0; y < m_header.sizeY; y++)
        {
            fwrite(&m_fields[x][y], sizeof(MapField), 1, f);
        }
    }

    fclose(f);
}

void Map::AddWorldObject(WorldObject* obj)
{
    if (m_objectGuidMap.find(obj->GetGUID()) != m_objectGuidMap.end())
        return;

    m_objectGuidMap[obj->GetGUID()] = obj;

    m_objectVisibilityVector.push_back(obj);
    obj->SetVisibilityIndex((uint32_t)m_objectVisibilityVector.size() - 1);
    CheckObjectVisibilityIndex(obj->GetVisibilityIndex());

    obj->OnAddedToMap();
}

void Map::RemoveWorldObject(WorldObject* obj)
{
    if (m_objectGuidMap.find(obj->GetGUID()) == m_objectGuidMap.end())
        return;

    ObjectGuidMap::iterator itr = m_objectGuidMap.find(obj->GetGUID());
    if (itr != m_objectGuidMap.end())
        m_objectGuidMap.erase(itr);

    RemoveObjectFromVisibilityVector(obj->GetVisibilityIndex());

    // clear hover from object, if marked
    if (sGameplay->GetHoverObject() == obj)
        sGameplay->SetHoverObject(nullptr);
}

void Map::RemoveWorldObject(uint64_t guid)
{
    ObjectGuidMap::iterator itr = m_objectGuidMap.find(guid);
    if (itr != m_objectGuidMap.end())
        RemoveWorldObject(m_objectGuidMap[guid]);
}

WorldObject* Map::GetWorldObject(uint64_t guid)
{
    ObjectGuidMap::iterator itr = m_objectGuidMap.find(guid);
    if (itr == m_objectGuidMap.end())
        return nullptr;

    return itr->second;
}

ObjectVector const& Map::GetObjectVisibilityVector()
{
    return m_objectVisibilityVector;
}

ObjectGuidMap const& Map::GetObjectGuidMap()
{
    return m_objectGuidMap;
}

void Map::CheckObjectVisibilityIndex(uint32_t visibilityIndex)
{
    if (m_objectVisibilityVector.size() <= 1)
        return;

    bool changed = false;
    uint32_t index = visibilityIndex;
    int8_t direction = 0; // 0 = no dir, -1 = to the left (further from screen, in back), +1 = to the right (closer to screen, on top)

    WorldObject* tmp;

    do
    {
        if (direction == 0)
        {
            // we are not the closest object...
            if (index != m_objectVisibilityVector.size() - 1)
            {
                // and the object on the right ("should be closer") is further
                if (m_objectVisibilityVector[index + 1]->GetPositionY() < m_objectVisibilityVector[index]->GetPositionY())
                {
                    // direction to the right ("closer")
                    direction = +1;
                    continue;
                }
            }

            // we are not the furthest object...
            if (index != 0)
            {
                // ...and object on the left ("should be further") is closer
                if (m_objectVisibilityVector[index - 1]->GetPositionY() > m_objectVisibilityVector[index]->GetPositionY())
                {
                    // direction to the left ("further")
                    direction = -1;
                    continue;
                }
            }

            // no changes needed
            break;
        }
        // to the right ("closer")
        else if (direction == +1)
        {
            // we reached the last element, or the following element is closer
            if (index == m_objectVisibilityVector.size() - 1 || m_objectVisibilityVector[index + 1]->GetPositionY() > m_objectVisibilityVector[index]->GetPositionY())
                break;

            // swap objects in vector
            tmp = m_objectVisibilityVector[index + 1];
            m_objectVisibilityVector[index + 1] = m_objectVisibilityVector[index];
            m_objectVisibilityVector[index] = tmp;

            m_objectVisibilityVector[index]->SetVisibilityIndex(index);
            m_objectVisibilityVector[index + 1]->SetVisibilityIndex(index + 1);

            index++;
        }
        // to the left ("further")
        else
        {
            // we reached the first element, or the following element is further
            if (index == 0 || m_objectVisibilityVector[index - 1]->GetPositionY() < m_objectVisibilityVector[index]->GetPositionY())
                break;

            // swap objects in vector
            tmp = m_objectVisibilityVector[index - 1];
            m_objectVisibilityVector[index - 1] = m_objectVisibilityVector[index];
            m_objectVisibilityVector[index] = tmp;

            m_objectVisibilityVector[index]->SetVisibilityIndex(index);
            m_objectVisibilityVector[index - 1]->SetVisibilityIndex(index - 1);

            index--;
        }
    } while (true); // the loop is ended by inner "break" statement
}

void Map::RemoveObjectFromVisibilityVector(uint32_t visibilityIndex)
{
    // move all objects in visibility vector by one place left
    for (uint32_t i = visibilityIndex; i < m_objectVisibilityVector.size() - 1; i++)
    {
        m_objectVisibilityVector[i] = m_objectVisibilityVector[i + 1];
        m_objectVisibilityVector[i]->SetVisibilityIndex(i);
    }

    // resize vector; this will cut the last element
    m_objectVisibilityVector.resize(m_objectVisibilityVector.size() - 1);
}

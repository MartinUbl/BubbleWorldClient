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

#ifndef BW_MAP_H
#define BW_MAP_H

#include "MapEnums.h"

// force alignment to 4 bytes
#if defined(__GNUC__)
#pragma pack(4)
#else
#pragma pack(push,4)
#endif

/*
 * Map header structure
 */
struct MapHeader
{
    uint32_t mapVersionMagic;               // magic identifier
    uint32_t mapId;                         // ID of map
    char name[MAP_NAME_LENGTH];             // short name identifier, unused characters filled with zeroes
    uint32_t sizeX;                         // number of fields in X direction
    uint32_t sizeY;                         // number of fields in Y direction
    uint32_t entryX;                        // map entry point X coordinate
    uint32_t entryY;                        // map entry point Y coordinate
    uint16_t defaultFieldType;              // default field type
    uint32_t defaultFieldTexture;           // default field texture
    uint32_t defaultFieldFlags;             // default field flags
};

/*
 * Map field structure
 */
struct MapField
{
    uint16_t type;                          // field type
    uint32_t texture;                       // field texture
    uint32_t flags;                         // field flags
};

#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

typedef std::vector<MapField> MapFieldRow;
typedef std::vector<MapFieldRow> MapFieldMap;

class WorldObject;

typedef std::set<WorldObject*> ObjectSet;
typedef std::map<uint64_t, WorldObject*> ObjectGuidMap;

/*
 * Class representing map, its contents and methods related to object management
 */
class Map
{
    public:
        Map();
        virtual ~Map();

        // initializes empty map using header
        void InitEmpty(MapHeader &mh);
        // updates objects on map
        virtual void Update();
        // sets map ID
        void SetId(uint32_t id);
        // retrieves map ID
        uint32_t GetId();
        // sets field contents on specified location
        void SetFieldContents(uint32_t x, uint32_t y, uint16_t type, uint32_t texture, uint32_t flags);
        // retrieves map field pointer
        MapField* GetField(uint32_t x, uint32_t y);
        // retrieves map field pointer without any additional checks
        MapField const* GetField_unsafe(uint32_t x, uint32_t y) const;

        // retrieves chunk X index using starting coordinate
        static uint32_t GetChunkIndexX(uint32_t startX);
        // retrieves chunk Y index using starting coordinate
        static uint32_t GetChunkIndexY(uint32_t startY);
        // retrieves chunk starting X coordinate using index
        static uint32_t GetChunkStartX(uint32_t indexX);
        // retrieves chunk starting Y coordinate using index
        static uint32_t GetChunkStartY(uint32_t indexY);
        // retrieves sorrounding limits (considers map size)
        void GetCellSorroundingLimits(uint32_t cellX, uint32_t cellY, uint32_t &beginX, uint32_t &beginY, uint32_t &endX, uint32_t &endY);

        // loads map from file (specified in database)
        bool LoadFromFile();
        // saves map to file (specified in database)
        void SaveToFile();

        // adds object to map
        void AddWorldObject(WorldObject* obj);
        // removes object from map
        void RemoveWorldObject(WorldObject* obj);
        // removes object from map using its guid
        void RemoveWorldObject(uint64_t guid);
        // retrieves object from map using its guid
        WorldObject* GetWorldObject(uint64_t guid);

        // retrieves object set
        ObjectSet const& GetObjectSet();
        // retrieves object guid map
        ObjectGuidMap const& GetObjectGuidMap();

    protected:
        //

    private:
        // stored header
        MapHeader m_header;
        // map field contents
        MapFieldMap m_fields;

        // object set
        ObjectSet m_objectSet;
        // object guid map (key = guid)
        ObjectGuidMap m_objectGuidMap;
};

#endif

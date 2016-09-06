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

#ifndef BW_ITEMCACHESTORAGE_H
#define BW_ITEMCACHESTORAGE_H

#include "FileStorage.h"

/*
 * Structure containing cached item entry
 */
struct ItemCacheEntry
{
    // item ID
    uint32_t id;
    // item icon image ID
    uint32_t imageId;
    // item name
    std::wstring name;
    // description (may be empty)
    std::wstring description;
    // maximum number of this item per one slot
    uint32_t stackSize;
    // item rarity
    uint32_t rarity;

    // timestamp of entry creation
    uint32_t addedTimestamp;
};

typedef std::map<uint32_t, ItemCacheEntry> ItemCacheMap;

/*
 * Class used for maintaining image file storage
 */
class ItemCacheStorage : public FileStorage
{
    public:
        ItemCacheStorage();
        ~ItemCacheStorage();

        void CreateTablesIfNotExist();
        void Load();

        // adds new entry to item cache storage
        void AddItemCacheEntry(uint32_t id, uint32_t imageId, const wchar_t* name, const wchar_t* description, uint32_t stackSize, uint32_t rarity, uint32_t addedTimestamp);
        // retrieves item cached entry if exists
        ItemCacheEntry* GetItemCacheEntry(uint32_t id);

    protected:
        //

    private:
        // map of cached item entries
        ItemCacheMap m_itemCache;
};

#endif

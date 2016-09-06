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
#include "ItemCacheStorage.h"
#include "Log.h"

ItemCacheStorage::ItemCacheStorage() : FileStorage(SQLITE_DB_ITEMCACHE)
{
    //
}

ItemCacheStorage::~ItemCacheStorage()
{
    //
}

void ItemCacheStorage::CreateTablesIfNotExist()
{
    DBExecute("CREATE TABLE IF NOT EXISTS item_cache (id INTEGER, image_id INTEGER, name TEXT, description TEXT, stack_size INTEGER, rarity INTEGER, added_timestamp INTEGER);");
}

void ItemCacheStorage::Load()
{
    m_itemCache.clear();

    // load image records
    Query* qr = DBQuery("SELECT * FROM item_cache");

    uint32_t count = 0;
    uint32_t id;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            m_itemCache[id].id = id;
            m_itemCache[id].imageId = qr->getuval();
            m_itemCache[id].name = UTF8ToWString(qr->getstr());
            m_itemCache[id].description = UTF8ToWString(qr->getstr());
            m_itemCache[id].stackSize = qr->getuval();
            m_itemCache[id].rarity = qr->getuval();
            m_itemCache[id].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("ItemCacheStorage: Loaded %u cached items", count);
}

void ItemCacheStorage::AddItemCacheEntry(uint32_t id, uint32_t imageId, const wchar_t* name, const wchar_t* description, uint32_t stackSize, uint32_t rarity, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM item_cache WHERE id = %u", id);
    DBExecute("INSERT INTO item_cache (id, image_id, name, description, stack_size, rarity, added_timestamp) VALUES (%u, %u, '%s', '%s', %u, %u, %u);",
        id, imageId, WStringToUTF8(name).c_str(), WStringToUTF8(description).c_str(), stackSize, rarity, addedTimestamp);

    m_itemCache[id].id = id;
    m_itemCache[id].imageId = imageId;
    m_itemCache[id].name = name;
    m_itemCache[id].description = description;
    m_itemCache[id].stackSize = stackSize;
    m_itemCache[id].rarity = rarity;
    m_itemCache[id].addedTimestamp = addedTimestamp;
}

ItemCacheEntry* ItemCacheStorage::GetItemCacheEntry(uint32_t id)
{
    if (m_itemCache.find(id) == m_itemCache.end())
        return nullptr;

    return &m_itemCache[id];
}

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

#ifndef BW_STORAGE_MANAGER_H
#define BW_STORAGE_MANAGER_H

#include "Singleton.h"
#include "libsqlitewrapped.h"
#include "FileStorage.h"
#include "ImageStorage.h"
#include "MapStorage.h"
#include "ItemCacheStorage.h"

/*
 * Singleton class for managing any type of storage
 */
class StorageManager
{
    friend class Singleton<StorageManager>;
    public:
        ~StorageManager();

        // initializes storages and loads contents if needed
        bool Init();

        // retrieves file storage maintainer class
        FileStorage* GetFileStorage(FileDBStorageTypes type);
        // retrieves SQLite database maintainer class
        Database* GetFileDatabase(FileDBStorageTypes type);

    protected:
        // protected singleton constructor
        StorageManager();

    private:
        // file storage map
        FileStorageRecord m_fileDatabases[MAX_SQLITE_DB];
};

#define sStorageManager Singleton<StorageManager>::getInstance()
#define sImageStorage ((ImageStorage*)sStorageManager->GetFileStorage(SQLITE_DB_IMAGE))
#define sMapStorage ((MapStorage*)sStorageManager->GetFileStorage(SQLITE_DB_MAP))
#define sItemCache ((ItemCacheStorage*)sStorageManager->GetFileStorage(SQLITE_DB_ITEMCACHE))

#endif

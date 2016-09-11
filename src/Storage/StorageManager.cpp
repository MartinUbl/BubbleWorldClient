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
#include "Log.h"

#include "FileStorage.h"
#include "ImageStorage.h"
#include "MapStorage.h"
#include "ItemCacheStorage.h"
#include "InternalImageStorage.h"

// array of file storages to load
static FileDBStorageParams fileDatabases[MAX_SQLITE_DB] = {
    { "images.db", instantiateFileStorage<ImageStorage> },
    { "maps.db", instantiateFileStorage<MapStorage> },
    { "itemcache.db", instantiateFileStorage<ItemCacheStorage> },
    { "internal/internalimages.db", instantiateFileStorage<InternalImageStorage> }
};

StorageManager::StorageManager()
{
    //
}

StorageManager::~StorageManager()
{
    //
}

bool StorageManager::Init()
{
    // create data directory, if not exist
    if (!custom_createDirectory(DATA_DIR))
    {
        sLog->Error("StorageManager: could not create data directory " DATA_DIR);
        return false;
    }

    // initialize all file storages
    for (size_t i = 0; i < MAX_SQLITE_DB; i++)
    {
        sLog->Debug("StorageManager: opening database '%s'", fileDatabases[i].filename.c_str());

        // load database
        m_fileDatabases[i].database = new Database((std::string(DATA_DIR) + fileDatabases[i].filename).c_str());

        if (m_fileDatabases[i].database)
        {
            sLog->Debug("StorageManager: database '%s' opened successfully", fileDatabases[i].filename.c_str());
        }
        else
        {
            sLog->Error("StorageManager: could not open database '%s'", fileDatabases[i].filename.c_str());
            return false;
        }

        // instantiate
        m_fileDatabases[i].storage = fileDatabases[i].creator();
        // create nonexistant tables, upgrade databases, etc.
        m_fileDatabases[i].storage->CreateTablesIfNotExist();
        // load contents
        m_fileDatabases[i].storage->Load();
    }

    return true;
}

FileStorage* StorageManager::GetFileStorage(FileDBStorageTypes type)
{
    return m_fileDatabases[type].storage;
}

Database* StorageManager::GetFileDatabase(FileDBStorageTypes type)
{
    return m_fileDatabases[type].database;
}

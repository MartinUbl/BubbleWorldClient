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

#ifndef BW_FILESTORAGE_H
#define BW_FILESTORAGE_H

#include "libsqlitewrapped.h"

// file storages we currently use
enum FileDBStorageTypes
{
    SQLITE_DB_IMAGE = 0,
    SQLITE_DB_MAP = 1,
    SQLITE_DB_ITEMCACHE = 2,
    SQLITE_DB_INTERNAL_IMAGE = 3,
    MAX_SQLITE_DB
};

/*
 * Base class for all file storages
 */
class FileStorage
{
    public:
        // creates all tables if not already exist
        virtual void CreateTablesIfNotExist() = 0;
        // load contents to memory
        virtual void Load() = 0;

        // perform SQLite query
        Query* DBQuery(const char* query, ...);
        // execute SQLite query without result
        void DBExecute(const char* query, ...);

    protected:
        // protected constructor; instantiate child classes only
        FileStorage(FileDBStorageTypes type);

    private:
        // file storage type
        FileDBStorageTypes m_type;
};

typedef FileStorage* FileStorageCreator();
template <class T> FileStorage* instantiateFileStorage()
{
    return new T;
}

/*
 * Structure containing file storage parameters
 */
struct FileDBStorageParams
{
    // file storage filename
    std::string filename;
    // instantiating function
    FileStorageCreator* creator;
};

/*
 * Structure containing info about loaded file storage
 */
struct FileStorageRecord
{
    FileStorage* storage;
    Database* database;
};

#endif

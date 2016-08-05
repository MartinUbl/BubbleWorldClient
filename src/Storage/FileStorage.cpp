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
#include "FileStorage.h"
#include "StorageManager.h"
#include "Log.h"

FileStorage::FileStorage(FileDBStorageTypes type) : m_type(type)
{
    //
}

Query* FileStorage::DBQuery(const char* query, ...)
{
    Database* db = sStorageManager->GetFileDatabase(m_type);
    Query* qry = new Query(*db);

    char tmp[2048];

    va_list argList;
    va_start(argList, query);
    vsprintf(tmp, query, argList);
    va_end(argList);

    qry->get_result(tmp);

    if (qry->GetErrno() != 0 && qry->GetErrno() < SQLITE_ROW) // SQLITE_ROW and higher are considered positive result codes
        sLog->Error("SQLite Query Error (%i): %s", qry->GetErrno(), qry->GetError().c_str());

    return qry;
}

void FileStorage::DBExecute(const char* query, ...)
{
    Database* db = sStorageManager->GetFileDatabase(m_type);
    Query qry(*db);

    char tmp[2048];

    va_list argList;
    va_start(argList, query);
    vsprintf(tmp, query, argList);
    va_end(argList);

    qry.execute(tmp);

    if (qry.GetErrno() && qry.GetErrno() < SQLITE_ROW) // SQLITE_ROW and higher are considered positive result codes
        sLog->Error("SQLite Execute Error (%i): %s", qry.GetErrno(), qry.GetError().c_str());
}

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
#include "InternalImageStorage.h"
#include "Log.h"

InternalImageStorage::InternalImageStorage() : FileStorage(SQLITE_DB_INTERNAL_IMAGE)
{
    //
}

InternalImageStorage::~InternalImageStorage()
{
    //
}

void InternalImageStorage::CreateTablesIfNotExist()
{
    // create base images table
    DBExecute("CREATE TABLE IF NOT EXISTS images (id INTEGER, filename TEXT, checksum TEXT, added_time INTEGER);");
    // create image metadata table
    DBExecute("CREATE TABLE IF NOT EXISTS image_metadata (id INTEGER, size_x INTEGER, size_y INTEGER, base_center_x INTEGER, base_center_y INTEGER,\
        collision_x1 INTEGER, collision_y1 INTEGER, collision_x2 INTEGER, collision_y2 INTEGER, checksum TEXT, added_time INTEGER);");
    // create image animation metadata table
    DBExecute("CREATE TABLE IF NOT EXISTS image_anims (id INTEGER, anim_id INTEGER, frame_begin INTEGER, frame_end INTEGER, frame_delay INTEGER, added_time INTEGER);");
}

void InternalImageStorage::Load()
{
    m_imageRecords.clear();

    // load image records
    Query* qr = DBQuery("SELECT * FROM images");

    uint32_t count = 0;
    uint32_t id, animId;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            m_imageRecords[id].id = id;
            m_imageRecords[id].filename = qr->getstr();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("InternalImageStorage: Loaded %u cached images", count);

    // load image metadata records
    qr = DBQuery("SELECT * FROM image_metadata");

    count = 0;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            m_imageMetadata[id].id = id;
            m_imageMetadata[id].sizeX = qr->getuval();
            m_imageMetadata[id].sizeY = qr->getuval();
            m_imageMetadata[id].baseCenterX = qr->getuval();
            m_imageMetadata[id].baseCenterY = qr->getuval();
            m_imageMetadata[id].collisionX1 = qr->getuval();
            m_imageMetadata[id].collisionY1 = qr->getuval();
            m_imageMetadata[id].collisionX2 = qr->getuval();
            m_imageMetadata[id].collisionY2 = qr->getuval();

            m_imageMetadata[id].CalculateUnitCollisionBox();

            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("InternalImageStorage: Loaded %u cached image metadata", count);

    // load image animation records
    qr = DBQuery("SELECT * FROM image_anims");

    count = 0;

    if (qr->num_rows() > 0)
    {
        while (qr->fetch_row())
        {
            id = qr->getuval();
            animId = qr->getuval();
            m_imageMetadata[id].animations[animId].id = id;
            m_imageMetadata[id].animations[animId].animId = animId;
            m_imageMetadata[id].animations[animId].frameBegin = qr->getuval();
            m_imageMetadata[id].animations[animId].frameEnd = qr->getuval();
            m_imageMetadata[id].animations[animId].frameDelay = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("InternalImageStorage: Loaded %u cached image animations", count);
}

ImageDatabaseRecord* InternalImageStorage::GetImageRecord(uint32_t id)
{
    if (m_imageRecords.find(id) == m_imageRecords.end())
        return nullptr;
    return &m_imageRecords[id];
}

ImageMetadataDatabaseRecord* InternalImageStorage::GetImageMetadataRecord(uint32_t id)
{
    if (m_imageMetadata.find(id) == m_imageMetadata.end())
        return nullptr;
    return &m_imageMetadata[id];
}

ImageAnimationDatabaseRecord* InternalImageStorage::GetImageAnimationRecord(uint32_t id, uint32_t animId)
{
    if (m_imageMetadata.find(id) == m_imageMetadata.end())
        return nullptr;

    if (m_imageMetadata[id].animations.find(animId) == m_imageMetadata[id].animations.end())
        return nullptr;

    return &m_imageMetadata[id].animations[animId];
}

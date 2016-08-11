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
#include "ImageStorage.h"
#include "Log.h"

ImageStorage::ImageStorage() : FileStorage(SQLITE_DB_IMAGE)
{
    //
}

ImageStorage::~ImageStorage()
{
    //
}

void ImageStorage::CreateTablesIfNotExist()
{
    // create base images table
    DBExecute("CREATE TABLE IF NOT EXISTS images (id INTEGER, filename TEXT, checksum TEXT, added_time INTEGER);");
    // create image metadata table
    DBExecute("CREATE TABLE IF NOT EXISTS image_metadata (id INTEGER, size_x INTEGER, size_y INTEGER, base_center_x INTEGER, base_center_y INTEGER,\
        collision_x1 INTEGER, collision_y1 INTEGER, collision_x2 INTEGER, collision_y2 INTEGER, checksum TEXT, added_time INTEGER);");
    // create image animation metadata table
    DBExecute("CREATE TABLE IF NOT EXISTS image_anims (id INTEGER, anim_id INTEGER, frame_begin INTEGER, frame_end INTEGER, frame_delay INTEGER, added_time INTEGER);");
}

void ImageStorage::Load()
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
            m_imageRecords[id].checksumStr = qr->getstr();
            m_imageRecords[id].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("ImageStorage: Loaded %u cached images", count);

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
            m_imageMetadata[id].checksum = qr->getstr();
            m_imageMetadata[id].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("ImageStorage: Loaded %u cached image metadata", count);

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
            m_imageMetadata[id].animations[animId].addedTimestamp = qr->getuval();
            count++;
        }
    }

    qr->free_result();
    delete qr;

    sLog->Info("ImageStorage: Loaded %u cached image animations", count);
}

void ImageStorage::InsertImageRecord(uint32_t id, const char* name, const char* checksumStr, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM images WHERE id = %u", id);
    DBExecute("INSERT INTO images (id, filename, checksum, added_time) VALUES (%u, '%s', '%s', %u);", id, name, checksumStr, addedTimestamp);

    m_imageRecords[id].id = id;
    m_imageRecords[id].filename = name;
    m_imageRecords[id].checksumStr = checksumStr;
    m_imageRecords[id].addedTimestamp = addedTimestamp;
}

ImageDatabaseRecord* ImageStorage::GetImageRecord(uint32_t id)
{
    if (m_imageRecords.find(id) == m_imageRecords.end())
        return nullptr;
    return &m_imageRecords[id];
}

void ImageStorage::InsertImageMetadataRecord(uint32_t id, uint32_t sizeX, uint32_t sizeY, uint32_t baseCenterX, uint32_t baseCenterY, uint32_t collisionX1, uint32_t collisionY1, uint32_t collisionX2, uint32_t collisionY2, const char* checksumStr, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM image_metadata WHERE id = %u", id);
    DBExecute("INSERT INTO image_metadata (id, size_x, size_y, base_center_x, base_center_y, collision_x1, collision_y1, collision_x2, collision_y2, checksum, added_time) VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %u);",
        id, sizeX, sizeY, baseCenterX, baseCenterY, collisionX1, collisionY1, collisionX2, collisionY2, checksumStr, addedTimestamp);

    m_imageMetadata[id].id = id;
    m_imageMetadata[id].sizeX = sizeX;
    m_imageMetadata[id].sizeY = sizeY;
    m_imageMetadata[id].baseCenterX = baseCenterX;
    m_imageMetadata[id].baseCenterY = baseCenterY;
    m_imageMetadata[id].collisionX1 = collisionX1;
    m_imageMetadata[id].collisionY1 = collisionY1;
    m_imageMetadata[id].collisionX2 = collisionX2;
    m_imageMetadata[id].collisionY2 = collisionY2;
    m_imageMetadata[id].checksum = checksumStr;
    m_imageMetadata[id].addedTimestamp = addedTimestamp;
}

ImageMetadataDatabaseRecord* ImageStorage::GetImageMetadataRecord(uint32_t id)
{
    if (m_imageMetadata.find(id) == m_imageMetadata.end())
        return nullptr;
    return &m_imageMetadata[id];
}

void ImageStorage::InsertImageAnimationRecord(uint32_t id, uint32_t animId, uint32_t frameBegin, uint32_t frameEnd, uint32_t frameDelay, uint32_t addedTimestamp)
{
    DBExecute("DELETE FROM image_anims WHERE id = %u AND anim_id = %u", id, animId);
    DBExecute("INSERT INTO image_anims (id, anim_id, frame_begin, frame_end, frame_delay, added_time) VALUES (%u, %u, %u, %u, %u, %u);", id, animId, frameBegin, frameEnd, frameDelay, addedTimestamp);

    if (m_imageMetadata.find(id) == m_imageMetadata.end())
    {
        m_imageMetadata[id].id = id;
        m_imageMetadata[id].addedTimestamp = 0;
    }

    m_imageMetadata[id].animations[animId].id = id;
    m_imageMetadata[id].animations[animId].animId = animId;
    m_imageMetadata[id].animations[animId].frameBegin = frameBegin;
    m_imageMetadata[id].animations[animId].frameEnd = frameEnd;
    m_imageMetadata[id].animations[animId].frameDelay = frameDelay;
    m_imageMetadata[id].animations[animId].addedTimestamp = addedTimestamp;
}

ImageAnimationDatabaseRecord* ImageStorage::GetImageAnimationRecord(uint32_t id, uint32_t animId)
{
    if (m_imageMetadata.find(id) == m_imageMetadata.end())
        return nullptr;

    if (m_imageMetadata[id].animations.find(animId) == m_imageMetadata[id].animations.end())
        return nullptr;

    return &m_imageMetadata[id].animations[animId];
}

void ImageStorage::WipeImageMetadata(uint32_t id)
{
    DBExecute("DELETE FROM image_anims WHERE id = %u", id);
    DBExecute("DELETE FROM image_metadata WHERE id = %u", id);
}

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

#ifndef BW_IMAGESTORAGE_H
#define BW_IMAGESTORAGE_H

#include "FileStorage.h"

/*
 * Structure containing image database record values
 */
struct ImageDatabaseRecord
{
    // image ID
    uint32_t id;
    // image filename
    std::string filename;
    // checksum
    std::string checksumStr;
    // timestamp of record creation
    uint32_t addedTimestamp;
};

typedef std::unordered_map<uint32_t, ImageDatabaseRecord> ImageRecordMap;

/*
 * Structure containing info about image animation
 */
struct ImageAnimationDatabaseRecord
{
    // image ID
    uint32_t id;
    // animation ID
    uint32_t animId;
    // first frame of animation
    uint32_t frameBegin;
    // last frame of animation
    uint32_t frameEnd;
    // delay between frames
    uint32_t frameDelay;
    // timestamp of record creation
    uint32_t addedTimestamp;
};

typedef std::unordered_map<uint32_t, ImageAnimationDatabaseRecord> ImageAnimationRecordMap;

/*
 * Structure containing image metadata
 */
struct ImageMetadataDatabaseRecord
{
    // image ID
    uint32_t id;
    // frame width
    uint32_t sizeX;
    // frame height
    uint32_t sizeY;
    // center image to this point (X coordinate)
    uint32_t baseCenterX;
    // center image to this point (Y coordinate)
    uint32_t baseCenterY;
    // checksum
    std::string checksum;
    // timestamp of record creation
    uint32_t addedTimestamp;

    // loaded animation
    ImageAnimationRecordMap animations;
};

typedef std::unordered_map<uint32_t, ImageMetadataDatabaseRecord> ImageMetaRecordMap;

/*
 * Class used for maintaining image file storage
 */
class ImageStorage : public FileStorage
{
    public:
        ImageStorage();
        ~ImageStorage();

        void CreateTablesIfNotExist();
        void Load();

        // inserts new image record
        void InsertImageRecord(uint32_t id, const char* name, const char* checksumStr, uint32_t addedTimestamp);
        // retrieves image record
        ImageDatabaseRecord* GetImageRecord(uint32_t id);
        // inserts image metadata record
        void InsertImageMetadataRecord(uint32_t id, uint32_t sizeX, uint32_t sizeY, uint32_t baseCenterX, uint32_t baseCenterY, const char* checksumStr, uint32_t addedTimestamp);
        // retrieves image metadata record
        ImageMetadataDatabaseRecord* GetImageMetadataRecord(uint32_t id);
        // inserts image animation record
        void InsertImageAnimationRecord(uint32_t id, uint32_t animId, uint32_t frameBegin, uint32_t frameEnd, uint32_t frameDelay, uint32_t addedTimestamp);
        // retrieves image animation record
        ImageAnimationDatabaseRecord* GetImageAnimationRecord(uint32_t id, uint32_t animId);
        // wipes all image metadata including animations
        void WipeImageMetadata(uint32_t id);

    protected:
        //

    private:
        // map of image records
        ImageRecordMap m_imageRecords;
        // map of image metadata
        ImageMetaRecordMap m_imageMetadata;
};

#endif

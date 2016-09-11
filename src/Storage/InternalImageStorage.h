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

#ifndef BW_INTERNALIMAGESTORAGE_H
#define BW_INTERNALIMAGESTORAGE_H

#include "FileStorage.h"
#include "ImageStorage.h"
#include "Drawing.h"

/*
 * Class used for maintaining internal image file storage
 */
class InternalImageStorage : public FileStorage
{
    public:
        InternalImageStorage();
        ~InternalImageStorage();

        void CreateTablesIfNotExist();
        void Load();

        // retrieves image record
        ImageDatabaseRecord* GetImageRecord(uint32_t id);
        // retrieves image metadata record
        ImageMetadataDatabaseRecord* GetImageMetadataRecord(uint32_t id);
        // retrieves image animation record
        ImageAnimationDatabaseRecord* GetImageAnimationRecord(uint32_t id, uint32_t animId);

    protected:
        //

    private:
        // map of image records
        ImageRecordMap m_imageRecords;
        // map of image metadata
        ImageMetaRecordMap m_imageMetadata;
};

#endif

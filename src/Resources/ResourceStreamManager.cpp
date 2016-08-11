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
#include "ResourceManager.h"
#include "ResourceStreamManager.h"
#include "StorageManager.h"
#include "CRC32.h"
#include "Log.h"
#include "SmartPacket.h"
#include "NetworkManager.h"

#include <iomanip>
#include <sstream>

ResourceStreamManager::ResourceStreamManager()
{
    //
}

ResourceStreamManager::~ResourceStreamManager()
{
    //
}

void ResourceStreamManager::CreateResourceStream(const char* filename, ResourceType type, uint32_t id)
{
    ResourceStreamRecord* rs = new ResourceStreamRecord();

    rs->id = id;
    rs->type = type;
    rs->filename = filename;
    rs->fileStream = new std::ofstream((std::string(DATA_DIR) + std::string(filename)).c_str(), std::ios::binary);
    rs->filePosition = 0;

    uint64_t pos = MAKE_RES_PAIR(id, type);
    m_resourceStreams[pos] = rs;
}

void ResourceStreamManager::WriteToResourceStream(ResourceType type, uint32_t id, uint16_t size, uint8_t* data)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    ResourceStreamRecord* rs = m_resourceStreams[pos];

    rs->fileStream->write((const char*)data, size);

    rs->filePosition += size;
}

void ResourceStreamManager::FinishResourceStream(ResourceType type, uint32_t id)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    ResourceStreamRecord* rs = m_resourceStreams[pos];

    rs->fileStream->close();
    delete rs->fileStream;

    // open file for reading to calculate checksum
    FILE* f = fopen((std::string(DATA_DIR) + rs->filename).c_str(), "rb");

    // calculate CRC32 checksum of resource file
    uint32_t crc = CRC32_File(f);

    rs->checksumStr = GetCRC32String(crc).c_str();

    // finally insert resource record to local file storage
    switch (type)
    {
        case RSTYPE_IMAGE:
            sImageStorage->InsertImageRecord(id, rs->filename.c_str(), rs->checksumStr.c_str(), (uint32_t)time(nullptr));
            break;
        default:
            sLog->Error("ResourceStreamManager: could not save resource type %u, id %u - unknown resource type or storage not available", type, id);
            break;
    }

    // erase resource stream, cleanup
    m_resourceStreams.erase(pos);
    delete rs;

    // signal resource manager to load newly retrieved resource
    sResourceManager->SignalResourceRetrieved(type, id);
}

void ResourceStreamManager::SendVerifyChecksumPacket(ResourceType type, uint32_t id, const char* checksum)
{
    // if checksum already verified, do not verify again
    if (IsResourceChecksumVerified(type, id))
        return;

    SmartPacket pkt(CP_RESOURCE_VERIFY_CHECKSUM);
    pkt.WriteUInt16(1);
    pkt.WriteUInt8(type);
    pkt.WriteUInt32(id);
    pkt.WriteString(checksum);
    sNetwork->SendPacket(pkt);
}

void ResourceStreamManager::SendVerifyChecksumsPacket(std::list<ResourceChecksumContainer> &resList)
{
    std::list<ResourceChecksumContainer>::iterator itr = resList.begin();

    uint16_t cnt;
    for (int i = 0; i < resList.size(); i++)
    {
        SmartPacket pkt(CP_RESOURCE_VERIFY_CHECKSUM);
        cnt = num_min(MAX_CHECKSUM_VERIFY_ITEMS, (uint16_t) resList.size() - 1 - i);
        pkt.WriteUInt16(cnt);

        for (int j = 0; j < cnt; j++)
        {
            pkt.WriteUInt8((*itr).type);
            pkt.WriteUInt32((*itr).id);
            pkt.WriteString((*itr).checksum);
            ++itr;
        }

        i += cnt;

        sNetwork->SendPacket(pkt);
    }
}

void ResourceStreamManager::SendVerifyMetadataChecksumPacket(ResourceType type, uint32_t id, const char* checksum)
{
    // if checksum already verified, do not verify again
    if (IsMetadataChecksumVerified(type, id))
        return;

    if (type == RSTYPE_IMAGE)
    {
        SmartPacket pkt(CP_VERIFY_IMAGE_METADATA_CHECKSUM);
        pkt.WriteUInt32(id);
        pkt.WriteString(checksum);
        sNetwork->SendPacket(pkt);
    }
}

void ResourceStreamManager::SetResourceChecksumVerified(ResourceType type, uint32_t id)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    m_resourceChecksumsVerified[pos] = true;
}

void ResourceStreamManager::SetMetadataChecksumVerified(ResourceType type, uint32_t id)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    m_metadataChecksumsVerified[pos] = true;
}

bool ResourceStreamManager::IsResourceChecksumVerified(ResourceType type, uint32_t id)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    if (m_resourceChecksumsVerified.find(pos) == m_resourceChecksumsVerified.end())
        return false;

    return m_resourceChecksumsVerified[pos];
}

bool ResourceStreamManager::IsMetadataChecksumVerified(ResourceType type, uint32_t id)
{
    uint64_t pos = MAKE_RES_PAIR(id, type);
    if (m_metadataChecksumsVerified.find(pos) == m_metadataChecksumsVerified.end())
        return false;

    return m_metadataChecksumsVerified[pos];
}

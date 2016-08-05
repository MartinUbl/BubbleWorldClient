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

#ifndef BW_RESOURCE_STREAM_MANAGER
#define BW_RESOURCE_STREAM_MANAGER

#include "Singleton.h"

#include <fstream>

/*
 * Structure containing info about resource stream
 */
struct ResourceStreamRecord
{
    // resource type
    ResourceType type;
    // resource ID
    uint32_t id;
    // resource filename
    std::string filename;
    // resource checksum
    std::string checksumStr;
    // current file position
    uint32_t filePosition;
    // opened file output stream
    std::ofstream* fileStream;
};

/*
 * Structure containing resource checksum info
 */
struct ResourceChecksumContainer
{
    // resource type
    ResourceType type;
    // resource ID
    uint32_t id;
    // calculated checksum
    const char* checksum;
};

#define MAKE_RES_PAIR(a,b) ((uint64_t)((uint64_t)a << 32 | b));

// maximum number of resources in checksum verify packet
#define MAX_CHECKSUM_VERIFY_ITEMS 50

class ResourceStreamManager
{
    friend class Singleton<ResourceStreamManager>;
    public:
        ~ResourceStreamManager();

        // creates new resource stream record
        void CreateResourceStream(const char* filename, ResourceType type, uint32_t id);
        // writes data into opened resource stream
        void WriteToResourceStream(ResourceType type, uint32_t id, uint16_t size, uint8_t* data);
        // finishes resource stream, closes file
        void FinishResourceStream(ResourceType type, uint32_t id);

        // sends packet to verify checksum of single resource
        void SendVerifyChecksumPacket(ResourceType type, uint32_t id, const char* checksum);
        // sends packet to verify checksums of multiple resources
        void SendVerifyChecksumsPacket(std::list<ResourceChecksumContainer> &resList);
        // sends packet to verify metadata checksum of single resource
        void SendVerifyMetadataChecksumPacket(ResourceType type, uint32_t id, const char* checksum);

        // sets flag to not verify resource checksum during this run again
        void SetResourceChecksumVerified(ResourceType type, uint32_t id);
        // sets flag to not verify resource metadata checksum during this run again
        void SetMetadataChecksumVerified(ResourceType type, uint32_t id);
        // was resource checksum already verified?
        bool IsResourceChecksumVerified(ResourceType type, uint32_t id);
        // was resource metadata checksum already verified?
        bool IsMetadataChecksumVerified(ResourceType type, uint32_t id);

    protected:
        // protected singleton constructor
        ResourceStreamManager();

    private:
        // opened and running resource streams
        std::map<uint64_t, ResourceStreamRecord*> m_resourceStreams;
        // map of verified resource checksums
        std::map<uint64_t, bool> m_resourceChecksumsVerified;
        // map of verified resource metadata checksums
        std::map<uint64_t, bool> m_metadataChecksumsVerified;
};

#define sResourceStreamManager Singleton<ResourceStreamManager>::getInstance()

#endif

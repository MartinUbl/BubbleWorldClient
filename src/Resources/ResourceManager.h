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

#ifndef BW_RESOURCEMANAGER_H
#define BW_RESOURCEMANAGER_H

#include "Singleton.h"

// load state of resource
enum ResourceLoadState
{
    RLS_NOT_LOADED = 1,             // not loaded, no attempt to load
    RLS_RETRIEVING = 2,             // resource is being retrieved from remote server
    RLS_LOADING_FROM_FILE = 3,      // resource is being loaded from file
    RLS_LOADED = 4,                 // resource is loaded and directly available
    MAX_RLS
};

/*
 * Structure for storing general information about resource
 */
struct BaseResource
{
    BaseResource(ResourceType _type, uint32_t _id = 0) : type(_type), id(_id) { };

    // resource ID
    uint32_t id;
    // resource type
    ResourceType type;
    // load state of contents
    ResourceLoadState loadState;
    // load state of metadata
    ResourceLoadState metaLoadState;
};

struct ImageMetadataDatabaseRecord;

/*
 * Structure for storing image resource data
 */
struct ImageResource : public BaseResource
{
    ImageResource(uint32_t _id = 0) : BaseResource(RSTYPE_IMAGE, _id), isInternal(false) { };

    // prerendered texture
    SDL_Texture* renderedTexture;
    // metadata pointer
    ImageMetadataDatabaseRecord* metadata;
    // is internal resource?
    bool isInternal;
    // cached rectangles used for drawing different portions of sprite as animation frames
    std::vector<SDL_Rect> animSpriteRects;
};

typedef std::unordered_map<uint32_t, BaseResource*> ResourceMap;

/*
 * Singleton class for managing all available and needed resources during runtime
 */
class ResourceManager
{
    friend class Singleton<ResourceManager>;
    public:
        ~ResourceManager();

        // signals resource manager about finishing resource stream
        void SignalResourceRetrieved(ResourceType type, uint32_t id);
        // signals resource manager about resource metadata retrieval
        void SignalResourceMetadataRetrieved(ResourceType type, uint32_t id);
        // retrieves prerendered image
        SDL_Texture* GetImage(uint32_t id);
        // retrieves image metadata
        ImageMetadataDatabaseRecord* GetImageMetadata(uint32_t id);
        // retrieves whole image record
        ImageResource* GetImageRecord(uint32_t id);
        // retrieves prerendered internal image
        SDL_Texture* GetInternalImage(uint32_t id);
        // retrieves internal image metadata
        ImageMetadataDatabaseRecord* GetInternalImageMetadata(uint32_t id);
        // retrieves whole internal image record
        ImageResource* GetInternalImageRecord(uint32_t id);
        // requests resource from remote server
        void RequestResource(ResourceType type, uint32_t id);
        // requests resource metadata from remote server
        void RequestResourceMetadata(ResourceType type, uint32_t id);
        // retrieves resource file checksum of specified resource
        const char* GetResourceFileChecksum(ResourceType type, uint32_t id);

    protected:
        // protected singleton constructor
        ResourceManager();

        // load image resource, if not available, request it, if available, verify checksum
        void LoadImageResource(ImageResource* imgres);
        // load image resource metadata, if not available, request it, if available, verify checksum
        void LoadImageResourceMeta(ImageResource* imgres);
        // retrieve internal image resource record
        ImageResource* GetOrCreateImageResource(uint32_t id, bool isInternal = false);
        // prepare cached rectangles for animations of specified resource
        void CacheAnimSpriteRectagles(ImageResource* imgres);

    private:
        // resource map
        ResourceMap m_resources[MAX_RSTYPE];
        // internal resource map
        ResourceMap m_internalResources[MAX_RSTYPE];
};

#define sResourceManager Singleton<ResourceManager>::getInstance()

#endif

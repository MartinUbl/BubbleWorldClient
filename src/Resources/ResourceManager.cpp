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
#include "ImageStorage.h"
#include "StorageManager.h"
#include "Drawing.h"
#include "SmartPacket.h"
#include "NetworkManager.h"
#include "Log.h"
#include "ResourceStreamManager.h"

ResourceManager::ResourceManager()
{
    //
}

ResourceManager::~ResourceManager()
{
    //
}

void ResourceManager::SignalResourceRetrieved(ResourceType type, uint32_t id)
{
    // reset state to not loaded, to be able to perform generic loading process
    m_resources[RSTYPE_IMAGE][id]->loadState = RLS_NOT_LOADED;

    // store flag to not verify checksum again during current gameplay
    sResourceStreamManager->SetResourceChecksumVerified(type, id);

    switch (type)
    {
        case RSTYPE_IMAGE:
            LoadImageResource((ImageResource*)m_resources[RSTYPE_IMAGE][id]);
            sDrawing->SetUIRedrawFlag();
            sDrawing->SetCanvasRedrawFlag();
            break;
        default:
            break;
    }
}

void ResourceManager::SignalResourceMetadataRetrieved(ResourceType type, uint32_t id)
{
    // reset state to not loaded, to be able to perform generic loading process
    m_resources[RSTYPE_IMAGE][id]->metaLoadState = RLS_NOT_LOADED;

    // store flag to not verify checksum again during current gameplay
    sResourceStreamManager->SetMetadataChecksumVerified(type, id);

    switch (type)
    {
        case RSTYPE_IMAGE:
            LoadImageResourceMeta((ImageResource*)m_resources[RSTYPE_IMAGE][id]);
            sDrawing->SetUIRedrawFlag();
            sDrawing->SetCanvasRedrawFlag();
            break;
        default:
            break;
    }
}

ImageResource* ResourceManager::GetOrCreateImageResource(uint32_t id)
{
    ImageResource* imgres;

    // if resource does not exist, create it
    if (m_resources[RSTYPE_IMAGE].find(id) == m_resources[RSTYPE_IMAGE].end())
    {
        imgres = new ImageResource(id);
        imgres->renderedTexture = nullptr;
        imgres->metadata = nullptr;
        m_resources[RSTYPE_IMAGE][id] = imgres;
        m_resources[RSTYPE_IMAGE][id]->loadState = RLS_NOT_LOADED;
        m_resources[RSTYPE_IMAGE][id]->metaLoadState = RLS_NOT_LOADED;
    }
    else
        imgres = (ImageResource*)m_resources[RSTYPE_IMAGE][id];

    // if metadata not loaded, load it
    if (imgres->metaLoadState == RLS_NOT_LOADED)
        LoadImageResourceMeta(imgres);
    // if contents not loaded, load it
    if (imgres->loadState == RLS_NOT_LOADED)
        LoadImageResource(imgres);

    return imgres;
}

SDL_Texture* ResourceManager::GetImage(uint32_t id)
{
    ImageResource* imgres = GetOrCreateImageResource(id);
    return imgres->renderedTexture;
}

ImageMetadataDatabaseRecord* ResourceManager::GetImageMetadata(uint32_t id)
{
    ImageResource* imgres = GetOrCreateImageResource(id);
    return imgres->metadata;
}

ImageResource* ResourceManager::GetImageRecord(uint32_t id)
{
    ImageResource* imgres = GetOrCreateImageResource(id);
    return imgres;
}

void ResourceManager::CacheAnimSpriteRectagles(ImageResource* imgres)
{
    // metadata has to be loaded, as well, as texture itself
    if (imgres->metaLoadState != RLS_LOADED || !imgres->metadata || !imgres->renderedTexture)
        return;

    uint32_t format;
    int acc, w, h;
    // retrieve texture dimensions
    if (SDL_QueryTexture(imgres->renderedTexture, &format, &acc, &w, &h) != 0)
        return;

    // calculate counts
    uint32_t countX = w / imgres->metadata->sizeX;
    uint32_t countY = h / imgres->metadata->sizeY;

    // has to have at least one anim frame
    if (countX == 0)
        countX = 1;
    if (countY == 0)
        countY = 1;

    uint32_t curr = 0;

    // clear and resize vector
    imgres->animSpriteRects.clear();
    imgres->animSpriteRects.resize(countX * countY);

    // split sprite to rectangles by rows
    // the texture is cut as follows:
    /*
    -------------------
    | 0| 1| 2| 3| 4| 5|
    |-----------------|
    | 6| 7| 8| 9|10|11|
    -------------------
    */
    for (uint32_t iY = 0; iY < countY; iY++)
    {
        for (uint32_t iX = 0; iX < countX; iX++)
        {
            imgres->animSpriteRects[curr].x = iX * imgres->metadata->sizeX;
            imgres->animSpriteRects[curr].y = iY * imgres->metadata->sizeY;
            imgres->animSpriteRects[curr].w = imgres->metadata->sizeX;
            imgres->animSpriteRects[curr].h = imgres->metadata->sizeY;

            curr++;
        }
    }
}

void ResourceManager::LoadImageResourceMeta(ImageResource* imgres)
{
    // loading should not already be in progress
    if (imgres->metaLoadState != RLS_RETRIEVING && imgres->metaLoadState != RLS_LOADING_FROM_FILE)
    {
        ImageMetadataDatabaseRecord* rec = sImageStorage->GetImageMetadataRecord(imgres->id);
        // we know about this image
        if (rec)
        {
            imgres->metaLoadState = RLS_LOADED;
            imgres->metadata = rec;

            CacheAnimSpriteRectagles(imgres);

            sDrawing->SetCanvasRedrawFlag();

            sResourceStreamManager->SendVerifyMetadataChecksumPacket(RSTYPE_IMAGE, imgres->id, rec->checksum.c_str());
        }
        else
            RequestResourceMetadata(RSTYPE_IMAGE, imgres->id);
    }
}

void ResourceManager::LoadImageResource(ImageResource* imgres)
{
    // loading should not already be in progress (also prohibit state: loaded + metadata loading)
    if (imgres->loadState != RLS_RETRIEVING && imgres->loadState != RLS_LOADING_FROM_FILE && !(imgres->loadState == RLS_LOADED && imgres->metaLoadState == RLS_RETRIEVING))
    {
        ImageDatabaseRecord* rec = sImageStorage->GetImageRecord(imgres->id);
        // we know about this resource
        if (rec)
        {
            imgres->loadState = RLS_LOADING_FROM_FILE;
            SDL_Surface* imgsurface = IMG_Load((std::string(DATA_DIR) + rec->filename).c_str());
            if (imgsurface)
            {
                if (imgres->renderedTexture)
                {
                    // TODO: this should be atomic; please, after reworking loading procedure
                    // to threaed stuff, remember to put lock here, thanks
                    SDL_DestroyTexture(imgres->renderedTexture);
                    imgres->renderedTexture = nullptr;
                }
                imgres->renderedTexture = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), imgsurface);
                imgres->loadState = RLS_LOADED;

                SDL_FreeSurface(imgsurface);

                CacheAnimSpriteRectagles(imgres);

                // signal drawing class to redraw all visible stuff
                sDrawing->SetCanvasRedrawFlag();
            }
            else
            {
                sLog->Error("Could not load image (ID: %u) from file %s", imgres->id, rec->filename.c_str());
            }

            // i know i would read this later, so.. my idea is to draw the old resource,
            // send checksum verify packet and if the checksum does not match, request the
            // resource once again, drop old texture, load new texture and redraw
            sResourceStreamManager->SendVerifyChecksumPacket(RSTYPE_IMAGE, imgres->id, rec->checksumStr.c_str());
        }
        else
        {
            // this will send request packet and set state to "retrieving"
            // the rest is handled by NetworkHandlers and ResourceStreamManager
            RequestResource(RSTYPE_IMAGE, imgres->id);
        }
    }
}

void ResourceManager::RequestResource(ResourceType type, uint32_t id)
{
    SmartPacket pkt(CP_REQUEST_RESOURCE);
    pkt.WriteUInt8(type);
    pkt.WriteUInt32(id);
    sNetwork->SendPacket(pkt);

    m_resources[RSTYPE_IMAGE][id]->loadState = RLS_RETRIEVING;
}

void ResourceManager::RequestResourceMetadata(ResourceType type, uint32_t id)
{
    if (type == RSTYPE_IMAGE)
    {
        SmartPacket pkt(CP_GET_IMAGE_METADATA);
        pkt.WriteUInt32(id);
        sNetwork->SendPacket(pkt);

        m_resources[RSTYPE_IMAGE][id]->metaLoadState = RLS_RETRIEVING;
    }
}

const char* ResourceManager::GetResourceFileChecksum(ResourceType type, uint32_t id)
{
    switch (type)
    {
        case RSTYPE_IMAGE:
        {
            ImageDatabaseRecord* res = sImageStorage->GetImageRecord(id);
            if (!res)
                return nullptr;
            return res->checksumStr.c_str();
        }
        default:
            return nullptr;
    }
}

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
#include "WorldObject.h"
#include "Unit.h"
#include "Player.h"
#include "Creature.h"
#include "AnimEnums.h"
#include "ImageStorage.h"
#include "StorageManager.h"
#include "Drawing.h"
#include "Gameplay.h"
#include "Colors.h"
#include "Map.h"

WorldObject::WorldObject(ObjectType type) : m_position(0.0f, 0.0f), m_mapId(0), m_objectType(type)
{
    m_animId = -1;
    m_animFrame = 0;
    m_animTimer = getMSTime();

    m_name = "???";
    m_nameTexture = nullptr;
}

WorldObject::~WorldObject()
{
    //
}

uint64_t WorldObject::GetGUID()
{
    return GetUInt64Value(OBJECT_FIELD_GUID);
}

uint32_t WorldObject::GetEntry()
{
    return EXTRACT_ENTRY(GetGUID());
}

uint32_t WorldObject::GetGUIDLow()
{
    return EXTRACT_GUIDLOW(GetGUID());
}

ObjectType WorldObject::GetType()
{
    return m_objectType;
}

Unit* WorldObject::ToUnit()
{
    return dynamic_cast<Unit*>(this);
}

Player* WorldObject::ToPlayer()
{
    return dynamic_cast<Player*>(this);
}

Creature* WorldObject::ToCreature()
{
    return dynamic_cast<Creature*>(this);
}

void WorldObject::ApplyValueSet(uint8_t* values, uint32_t size)
{
    memcpy(m_updateFields, values, size);
}

void WorldObject::SetUInt32Value(uint32_t field, uint32_t value)
{
    if (m_updateFields[field] == value)
        return;

    m_updateFields[field] = value;
}

void WorldObject::SetUInt64Value(uint32_t field, uint64_t value)
{
    SetUInt32Value(field, value & 0xFFFFFFFF);
    SetUInt32Value(field + 1, (value >> 32LL) & 0xFFFFFFFF);
}

void WorldObject::SetUByteValue(uint32_t field, uint8_t offset, uint8_t value)
{
    // allowed offsets are 0-3, as there are 4 bytes in uint32_t
    if (offset > 3)
        return;

    if (m_updateFields[field] >> (offset * 8) == value)
        return;

    m_updateFields[field] = m_updateFields[field] & (~(0xFF << (offset * 8))) | (value << (offset * 8));
}

void WorldObject::SetInt32Value(uint32_t field, int32_t value)
{
    SetUInt32Value(field, (uint32_t)value);
}

void WorldObject::SetInt64Value(uint32_t field, int64_t value)
{
    SetUInt64Value(field, (uint64_t)value);
}

void WorldObject::SetByteValue(uint32_t field, uint8_t offset, int8_t value)
{
    SetUByteValue(field, offset, (uint8_t)value);
}

void WorldObject::SetFloatValue(uint32_t field, float value)
{
    SetUInt32Value(field, *((uint32_t*)(&value)));
}

uint32_t WorldObject::GetUInt32Value(uint32_t field)
{
    return m_updateFields[field];
}

uint64_t WorldObject::GetUInt64Value(uint32_t field)
{
    return ((uint64_t)m_updateFields[field]) | (((uint64_t)m_updateFields[field + 1]) << 32LL);
}

uint8_t WorldObject::GetUByteValue(uint32_t field, uint8_t offset)
{
    return m_updateFields[field] >> (offset * 8);
}

int32_t WorldObject::GetInt32Value(uint32_t field)
{
    return (int32_t)GetUInt32Value(field);
}

int64_t WorldObject::GetInt64Value(uint32_t field)
{
    return (int64_t)GetUInt64Value(field);
}

int8_t WorldObject::GetByteValue(uint32_t field, uint8_t offset)
{
    return (int8_t)GetUByteValue(field, offset);
}

float WorldObject::GetFloatValue(uint32_t field)
{
    uint32_t val = GetUInt32Value(field);
    return *((float*)(&val));
}

void WorldObject::SetName(const char* name)
{
    m_name = name;
    if (m_nameTexture)
    {
        SDL_DestroyTexture(m_nameTexture);
        m_nameTexture = nullptr;
    }
    sDrawing->SetCanvasRedrawFlag();
}

const char* WorldObject::GetName()
{
    return m_name.c_str();
}

SDL_Texture* WorldObject::GetNameTexture()
{
    // if the prerendered texture already exists, return it
    if (m_nameTexture)
        return m_nameTexture;

    SDL_Color col;
    switch (GetType())
    {
        case OTYPE_PLAYER:
            col = BWCOLOR_NAME_PLAYER;
            break;
        case OTYPE_CREATURE:
            // TODO: faction system, for now, 1 = universal friend, 2 = universal enemy
            if (GetUInt32Value(UNIT_FIELD_FACTION) == 1)
                col = BWCOLOR_NAME_NPC_FRIEND;
            else
                col = BWCOLOR_NAME_NPC_ENEMY;
            break;
        case OTYPE_GAMEOBJECT:
            col = BWCOLOR_NAME_GAMEOBJECT;
            break;
        default:
            col = defaultTextColor;
            break;
    }

    // render name texture
    SDL_Surface* tmp = sDrawing->RenderFont(FONT_NAME_TITLE, m_name.c_str(), col);
    if (tmp)
        m_nameTexture = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), tmp);

    // cleanup
    SDL_FreeSurface(tmp);

    sDrawing->SetCanvasRedrawFlag();

    return m_nameTexture;
}

void WorldObject::SetPosition(Position pos)
{
    m_position = pos;
}

void WorldObject::SetPosition(float x, float y)
{
    m_position.x = x;
    m_position.y = y;

    if (GetMap())
        GetMap()->CheckObjectVisibilityIndex(m_visibilityIndex);
}

void WorldObject::SetPositionX(float x)
{
    m_position.x = x;
}

void WorldObject::SetPositionY(float y)
{
    m_position.y = y;

    if (GetMap())
        GetMap()->CheckObjectVisibilityIndex(m_visibilityIndex);
}

Position const& WorldObject::GetPosition()
{
    return m_position;
}

float WorldObject::GetPositionX()
{
    return m_position.x;
}

float WorldObject::GetPositionY()
{
    return m_position.y;
}

void WorldObject::SetMapId(uint32_t mapId)
{
    m_mapId = mapId;
}

uint32_t WorldObject::GetMapId()
{
    return m_mapId;
}

Map* WorldObject::GetMap()
{
    return sGameplay->GetMap();
}

void WorldObject::InitializeObject(uint64_t guid)
{
    // at first, create updatefields - without them, we can do nothing!
    CreateUpdateFields();

    // set GUID
    SetUInt64Value(OBJECT_FIELD_GUID, guid);
}

void WorldObject::CreateUpdateFields()
{
    // derived classes will create its updatefields
}

uint32_t WorldObject::GetAnimTimer()
{
    return m_animTimer;
}

uint32_t WorldObject::GetAnimFrame()
{
    return m_animFrame;
}

void WorldObject::OnAddedToMap()
{
    SetAnimId(ANIM_IDLE);

    // ask server for name (or take one from cache)
    sGameplay->SendNameQuery(GetGUID());
}

void WorldObject::SetAnimId(uint32_t animId)
{
    // attempt to change animation to the same - do nothing
    if (m_animId == animId)
        return;

    // do we have texture?
    uint32_t textureId = GetUInt32Value(OBJECT_FIELD_IMAGEID);
    // if not, do not animate
    if (!textureId)
        return;

    // retrieve animation record
    ImageAnimationDatabaseRecord* animres = sImageStorage->GetImageAnimationRecord(textureId, animId);
    
    // IDLE animation may not have animation specified, but defaults to 0th frame
    if (animId == ANIM_IDLE)
    {
        // movement animations should "end" at their first frame when going idle
        if (_isMovementAnim(m_animId))
        {
            ImageAnimationDatabaseRecord* animres2 = sImageStorage->GetImageAnimationRecord(textureId, m_animId);
            if (animres2)
            {
                m_animFrame = animres2->frameBegin;
                m_animTimer = 0;
            }
            else
                m_animTimer = getMSTime();
        }
        else
        {
            if (animres)
                m_animFrame = animres->frameBegin;
            else
                m_animFrame = 0;

            m_animTimer = getMSTime();
        }

        m_animId = ANIM_IDLE;
        sDrawing->SetCanvasRedrawFlag();

        return;
    }

    // no anim? return
    if (!animres)
        return;

    m_animId = animId;
    // movement animations should skip their beginning frame - it's considered "idle" frame
    if (!_isMovementAnim(animId) || (animres->frameBegin + 1 >= animres->frameEnd))
        m_animFrame = animres->frameBegin;
    else
        m_animFrame = animres->frameBegin + 1;
    m_animTimer = getMSTime();
    sDrawing->SetCanvasRedrawFlag();
}

void WorldObject::Update()
{
    // update animation
    uint32_t textureId = GetUInt32Value(OBJECT_FIELD_IMAGEID);
    if (textureId)
    {
        ImageAnimationDatabaseRecord* animres = sImageStorage->GetImageAnimationRecord(textureId, m_animId);
        // if it's time to change animation frame...
        if (animres && m_animTimer && getMSTimeDiff(m_animTimer, getMSTime()) > animres->frameDelay)
        {
            // refresh timer, move frame by one
            m_animTimer = getMSTime();
            m_animFrame++;
            // and if we exceeded animation frame limit, loop
            if (m_animFrame > animres->frameEnd)
            {
                // movement animations should skip beginning frame
                if (!_isMovementAnim(m_animId) || (animres->frameBegin + 1 >= animres->frameEnd))
                    m_animFrame = animres->frameBegin;
                else
                    m_animFrame = animres->frameBegin + 1;
            }

            // redraw!
            sDrawing->SetCanvasRedrawFlag();
        }
    }
}

uint32_t WorldObject::GetVisibilityIndex() const
{
    return m_visibilityIndex;
}

void WorldObject::SetVisibilityIndex(uint32_t visibilityIndex)
{
    m_visibilityIndex = visibilityIndex;
}

void WorldObject::SetInView(bool state)
{
    m_isInView = state;
}

bool WorldObject::IsInView()
{
    return m_isInView;
}

SDL_Rect* WorldObject::GetViewRect()
{
    return &m_viewRect;
}

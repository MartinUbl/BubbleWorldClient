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
#include "Unit.h"
#include "AnimEnums.h"
#include "Drawing.h"
#include "Colors.h"
#include "Map.h"

Unit::Unit(ObjectType type) : WorldObject(type), m_moveVector(0.0f, 0.0f)
{
    m_moveMask = 0;
    m_displayChatHide = 0;
    m_displayChat = nullptr;
}

Unit::~Unit()
{
    //
}

void Unit::Update()
{
    WorldObject::Update();

    if (m_moveMask != 0)
    {
        uint32_t moveDiff = getMSTimeDiff(m_lastMovementUpdate, getMSTime());
        if (moveDiff >= 1)
        {
            // the vector is reduced to unit size, coefficient is "number of milliseconds passed"
            float coef = (float)moveDiff;
            // store old position
            float newX;
            float newY;

            // move on X axis
            newX = m_position.x + m_moveVector.x * coef;
            // secure boundaries
            if (newX < 0.0f)
                newX = 0.0f;

            // secure "walkable" types
            MapField* mf = GetMap()->GetField((uint32_t)newX, (uint32_t)m_position.y);
            if (!mf || !CanMoveOn((MapFieldType)mf->type, mf->flags))
                newX = m_position.x;

            SetPositionX(newX);

            // move on Y axis
            newY = m_position.y + m_moveVector.y * coef;
            // secure boundaries
            if (newY < 0.0f)
                newY = 0.0f;

            // secure "walkable" types
            mf = GetMap()->GetField((uint32_t)m_position.x, (uint32_t)newY);
            if (!mf || !CanMoveOn((MapFieldType)mf->type, mf->flags))
                newY = m_position.y;

            SetPositionY(newY);

            m_lastMovementUpdate = getMSTime();
            sDrawing->SetCanvasRedrawFlag();
        }
    }

    if (m_displayChat)
    {
        if (m_displayChatHide <= time(nullptr))
        {
            m_displayChatHide = 0;
            SDL_DestroyTexture(m_displayChat);
            m_displayChat = nullptr;
            sDrawing->SetCanvasRedrawFlag();
        }
    }
}

void Unit::InitializeObject(uint64_t guid)
{
    WorldObject::InitializeObject(guid);
}

void Unit::CreateUpdateFields()
{
    WorldObject::CreateUpdateFields();
}

bool Unit::CanMoveOn(MapFieldType type, uint32_t flags)
{
    // for now just ground type
    return (type == MFT_GROUND);
}

void Unit::OnMoveStart()
{
    m_lastMovementUpdate = getMSTime();
}

void Unit::OnMoveStop()
{
    //
}

void Unit::StartMovementInDirection(MoveDirectionElement dir)
{
    if ((m_moveMask & dir) != 0)
        return;

    bool startedMovement = false;
    if (m_moveMask == 0)
        startedMovement = true;

    m_moveMask |= dir;
    UpdateMovementVector();

    if (startedMovement)
        OnMoveStart();
}

void Unit::StopMovementInDirection(MoveDirectionElement dir)
{
    if ((m_moveMask & dir) == 0)
        return;

    m_moveMask &= ~dir;
    UpdateMovementVector();

    if (m_moveMask == 0)
        OnMoveStop();
}

bool Unit::IsMovingInDirection(MoveDirectionElement dir)
{
    return (m_moveMask & dir) != 0;
}

#define F_PI ((float)M_PI)

static const float movementAngles[] = {
    0.0f,           // 0 (none)
    F_PI * 1.5f,    // 1 (up)
    0.0f,           // 2 (right)
    F_PI * 1.75f,   // 3 (up + right)
    F_PI * 0.5f,    // 4 (down)
    0.0f,           // 5 (up + down = none)
    F_PI * 0.25f,   // 6 (right + down)
    0.0f,           // 7 (up + right + down = right)
    F_PI,           // 8 (left)
    F_PI * 1.25f,   // 9 (left + up)
    0.0f,           // 10 (left + right = none)
    F_PI * 1.5f,    // 11 (left + right + up = up)
    F_PI * 0.75f,   // 12 (left + down)
    F_PI,           // 13 (left + down + up = left)
    F_PI * 0.5f,    // 14 (left + down + right = down)
    0.0f            // 15 (all = none)
};

static const uint32_t movementAnims[] = {
    ANIM_IDLE,
    ANIM_WALK_UP,
    ANIM_WALK_RIGHT,
    ANIM_WALK_UPRIGHT,
    ANIM_WALK_DOWN,
    ANIM_IDLE,
    ANIM_WALK_DOWNRIGHT,
    ANIM_WALK_RIGHT,
    ANIM_WALK_LEFT,
    ANIM_WALK_UPLEFT,
    ANIM_IDLE,
    ANIM_WALK_UP,
    ANIM_WALK_DOWNLEFT,
    ANIM_WALK_LEFT,
    ANIM_WALK_DOWN,
    ANIM_IDLE
};

void Unit::UpdateMovementVector()
{
    if (m_moveMask % 5 == 0)
    {
        m_moveVector.x = 0;
        m_moveVector.y = 0;
    }
    else
    {
        m_moveVector.SetFromPolar(movementAngles[m_moveMask], GetFloatValue(UNIT_FIELD_MOVEMENT_SPEED));
        m_moveVector.x *= MOVEMENT_UPDATE_UNIT_FRACTION;
        m_moveVector.y *= MOVEMENT_UPDATE_UNIT_FRACTION;
    }

    SetAnimId(movementAnims[m_moveMask]);
}

SDL_Texture* Unit::GetDisplayChat()
{
    if (m_displayChatHide == 0 || m_displayChatHide <= time(nullptr) || !m_displayChat)
        return nullptr;
    return m_displayChat;
}

TalkType Unit::GetDisplayChatType()
{
    return m_displayChatTalkType;
}

void Unit::Talk(TalkType type, const char* str)
{
    m_displayChatTalkType = type;

    SDL_Surface* textsurf = sDrawing->RenderFontWrapped(FONT_CHAT, str, 200, BWCOLOR_BLUE);
    if (!textsurf)
        return;

    m_displayChat = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), textsurf);
    SDL_FreeSurface(textsurf);

    m_displayChatHide = time(nullptr) + 3;

    sDrawing->SetCanvasRedrawFlag();
}

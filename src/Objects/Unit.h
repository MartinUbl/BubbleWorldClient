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

#ifndef BW_UNIT_H
#define BW_UNIT_H

#include "WorldObject.h"
#include "Vector2.h"

enum MapFieldType;

// this is the number which we use to multiply movement vector
#define MOVEMENT_UPDATE_UNIT_FRACTION 0.001f

/*
 * Class for all "alive" objects in game (player, NPC)
 */
class Unit : public WorldObject
{
    public:
        virtual ~Unit();

        virtual void InitializeObject(uint64_t guid);
        virtual void Update();

        // called when movement starts (from stopped state)
        virtual void OnMoveStart();
        // called when movement completelly stops
        virtual void OnMoveStop();
        // starts movement in supplied direction, if not already moving that way
        virtual void StartMovementInDirection(MoveDirectionElement dir);
        // stops movement in supplied direction if moving that way
        virtual void StopMovementInDirection(MoveDirectionElement dir);
        // retrieves movement direction element state - is unit moving that way?
        virtual bool IsMovingInDirection(MoveDirectionElement dir);

        // retrieves texture for current chat bubble above unit, if any
        SDL_Texture* GetDisplayChat();
        // retrieves type of chat message
        TalkType GetDisplayChatType();
        // talks using specified type and supplied message
        void Talk(TalkType type, const wchar_t* str);
        // can the unit move over this field type?
        bool CanMoveOn(MapFieldType type, uint32_t flags);

    protected:
        // protected constructor; instantiate child classes only
        Unit(ObjectType type);
        virtual void CreateUpdateFields();

        // updates movement vector after moveMask change
        void UpdateMovementVector();

        // current movement mask (ORed movement direction elements)
        uint8_t m_moveMask;
        // calculated movement unit vector ("distance per millisecond")
        Vector2 m_moveVector;
        // last movement update time (mstime)
        uint32_t m_lastMovementUpdate;

        // chat bubble texture
        SDL_Texture* m_displayChat;
        // timestamp of chat bubble hide
        time_t m_displayChatHide;
        // type of chat message displayed
        TalkType m_displayChatTalkType;

    private:
        //
};

#endif

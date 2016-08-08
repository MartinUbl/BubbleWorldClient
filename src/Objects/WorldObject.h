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

#ifndef BW_WORLDOBJECT_H
#define BW_WORLDOBJECT_H

#include <math.h>
#include "UpdateFields.h"
#include "ObjectEnums.h"

/*
 * Position structure with base operations defined
 */
struct Position
{
    // base constructor, nullifying coordinates
    Position() : x(0.0f), y(0.0f) { };
    // constructor with coordinates initialization
    Position(float _x, float _y) : x(_x), y(_y) { };
    // copy constructor
    Position(const Position &pos) : x(pos.x), y(pos.y) { };

    // assignment operator copies coordinates from righthand operand
    Position& operator=(const Position &pos) { x = pos.x; y = pos.y; return *this; }
    // addition compount operator adds righthand operand coordinates
    Position& operator+=(const Position &pos) { x += pos.x; y += pos.y; return *this; }
    // subtraction compound operator subtracts operand coordinates
    Position& operator-=(const Position &pos) { x -= pos.x; y -= pos.y; return *this; }
    // addition operator adds coordinates of arguments and creates new position container
    friend Position operator+(const Position &posA, const Position &posB) { return Position(posA.x + posB.x, posA.y + posB.y); }
    // subtraction operator subtracts coordinates of arguments and creates new position container
    friend Position operator-(const Position &posA, const Position &posB) { return Position(posA.x - posB.x, posA.y - posB.y); }

    // calculates distance between two positions
    float GetDistance(const Position &pos) const
    {
        return sqrt((pos.x - x)*(pos.x - x) + (pos.y - y)*(pos.y - y));
    }

    // X coordinate
    float x;
    // Y coordinate
    float y;
};

class Unit;
class Player;
class Creature;
class Map;

/*
 * Class serving as base class for all objects in game
 */
class WorldObject
{
    public:
        virtual ~WorldObject();

        // initializes object, creates updatefields, sets guid
        virtual void InitializeObject(uint64_t guid);
        // updates object, internal timers, movement, ..
        virtual void Update();
        // after the object was added to map
        virtual void OnAddedToMap();

        // sets object name
        void SetName(const wchar_t* name);
        // retrieves object name
        const wchar_t* GetName();
        // retrieves prerendered name texture
        SDL_Texture* GetNameTexture();

        // sets position using Position container
        void SetPosition(Position pos);
        // sets position using coordinates
        void SetPosition(float x, float y);
        // sets position X coordinate
        void SetPositionX(float x);
        // sets position Y coordinate
        void SetPositionY(float y);
        // retrieves position
        Position const& GetPosition();
        // retrieves position X coordinate
        float GetPositionX();
        // retrieves position Y coordinate
        float GetPositionY();
        // sets current map ID
        void SetMapId(uint32_t mapId);
        // retrieves current map ID
        uint32_t GetMapId();
        // retrieves current map
        Map* GetMap();

        // retrieves object GUID
        uint64_t GetGUID();
        // retrieves object entry
        uint32_t GetEntry();
        // retrieves object low GUID
        uint32_t GetGUIDLow();
        // retrieves object type
        ObjectType GetType();

        // casts object to Unit class, if possible; otherwise returns nullptr
        Unit* ToUnit();
        // casts object to Player class, if possible; otherwise returns nullptr
        Player* ToPlayer();
        // casts object to Creature class, if possible; otherwise returns nullptr
        Creature* ToCreature();

        // overrides updatefield values with supplied
        void ApplyValueSet(uint8_t* values, uint32_t size);

        // sets 32bit unsigned field value
        void SetUInt32Value(uint32_t field, uint32_t value);
        // sets 64bit unsigned field value
        void SetUInt64Value(uint32_t field, uint64_t value);
        // sets unsigned byte field value
        void SetUByteValue(uint32_t field, uint8_t offset, uint8_t value);
        // sets 32bit signed field value
        void SetInt32Value(uint32_t field, int32_t value);
        // sets 64bit signed field value
        void SetInt64Value(uint32_t field, int64_t value);
        // sets signed byte field value
        void SetByteValue(uint32_t field, uint8_t offset, int8_t value);
        // sets float field value
        void SetFloatValue(uint32_t field, float value);
        // retrieves 32bit unsigned field value
        uint32_t GetUInt32Value(uint32_t field);
        // retrieves 64bit unsigned field value
        uint64_t GetUInt64Value(uint32_t field);
        // retrieves unsigned byte field value
        uint8_t GetUByteValue(uint32_t field, uint8_t offset);
        // retrieves 32bit signed field value
        int32_t GetInt32Value(uint32_t field);
        // retrieves 64bit signed field value
        int64_t GetInt64Value(uint32_t field);
        // retrieves signed byte field value
        int8_t GetByteValue(uint32_t field, uint8_t offset);
        // retrieves float field value
        float GetFloatValue(uint32_t field);

        // retrieves animation timer
        uint32_t GetAnimTimer();
        // retrieves current animation frame
        virtual uint32_t GetAnimFrame();
        // sets animation ID
        void SetAnimId(uint32_t animId);
        // retrieves visibility index
        uint32_t GetVisibilityIndex() const;
        // sets visibility index
        void SetVisibilityIndex(uint32_t visibilityIndex);

        // sets the "in view" flag
        void SetInView(bool state);
        // is the object in view?
        bool IsInView();
        // retrieves view rectangle
        SDL_Rect* GetViewRect();

    protected:
        // protected constructor; instantiate only child classes
        WorldObject(ObjectType type);
        // allocate update field space and nullify contents
        virtual void CreateUpdateFields();

        // object name
        std::wstring m_name;
        // object position
        Position m_position;
        // updatefields
        uint32_t* m_updateFields;
        // current map ID
        uint32_t m_mapId;
        // type of object
        ObjectType m_objectType;
        // current animation
        uint32_t m_animId;
        // current animation frame
        uint32_t m_animFrame;
        // current animation timer
        uint32_t m_animTimer;
        // is currently in view?
        bool m_isInView;
        // current view rectangle
        SDL_Rect m_viewRect;

    private:
        // prerendered name texture
        SDL_Texture* m_nameTexture;
        // object visibility index (index to m_objectVisibilityVector in Map class)
        uint32_t m_visibilityIndex;
};

#endif

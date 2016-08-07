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

#ifndef BW_APPLICATION_H
#define BW_APPLICATION_H

#include "Singleton.h"
#include "Stages.h"
#include "UI/UIEnums.h"

// enumerator of keyboard special mods
enum SpecialModKeys
{
    KEYMOD_SHIFT = 0,
    KEYMOD_CTRL = 1,
    KEYMOD_ALT = 2,
    KEYMOD_CAPS = 3,
    KEYMOD_NUM = 4,
    MAX_KEYMOD = 5
};

/*
 * Singleton class maintaining general stuff around application runtime
 */
class Application
{
    friend class Singleton<Application>;
    public:
        ~Application();

        // initializes application, parses configs, ..
        bool Init();
        // main run method
        int Run();

        // retrieves mouse X coordinate within window
        int32_t GetMouseX() { return m_mouseX; };
        // retrieves mouse Y coordinate within window
        int32_t GetMouseY() { return m_mouseY; };

        // sets stage type (real stage is updated in next update tick)
        void SetStageType(StageType type);
        // retrieves current stage type
        StageType GetStageType();

        // signals UI event to application
        void SignalUIEvent(uint32_t elementId, UIActionType actionId);
        // signals global event to application
        void SignalGlobalEvent(GlobalActionIDs actionId, void* actionParam = nullptr);

        // retrieves SHIFT key state
        bool GetShiftState() { return m_keyMod[KEYMOD_SHIFT]; };
        // retrieves CTRL key state
        bool GetCtrlState() { return m_keyMod[KEYMOD_CTRL]; };
        // retrieves ALT key state
        bool GetAltState() { return m_keyMod[KEYMOD_ALT]; };
        // retrieves CAPSLOCK state
        bool GetCapsLockState() { return m_keyMod[KEYMOD_CAPS]; };
        // retrieves NUMLOCK state
        bool GetNumLockState() { return m_keyMod[KEYMOD_NUM]; };

    protected:
        // protected singleton constructor
        Application();

        // internal method for switching stages
        void SetStage_internal();
        // stores keymod from key press event
        void StoreKeyMod(uint16_t mod);

    private:
        // mouse X coordinate
        int32_t m_mouseX;
        // mouse Y coordinate
        int32_t m_mouseY;
        // current stage
        StageTemplate* m_stage;
        // current stage type
        StageType m_stageType;
        // pending stage type (to be switched)
        StageType m_pendingStageType;

        // stored keymod
        bool m_keyMod[MAX_KEYMOD];
};

#define sApplication Singleton<Application>::getInstance()

#endif

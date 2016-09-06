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

#ifndef BW_STAGES_H
#define BW_STAGES_H

#include "UI/UIEnums.h"

// stage type enumerator
enum StageType
{
    STAGE_NONE = 0,
    STAGE_MENU = 1,
    STAGE_LOBBY = 2,
    STAGE_CONNECTING = 3,
    STAGE_GAME = 4,
    MAX_STAGE
};

/*
 * Base class for all stage classes
 */
class StageTemplate
{
    public:
        // called when application entered this stage
        virtual void OnEnter() { };
        // called when application leaves this stage
        virtual void OnLeave() { };
        // called before drawing everything
        virtual void OnBeforeDraw() { };
        // called after drawing everything
        virtual void OnDraw() { };
        // called when mouse clicks (press or release)
        virtual void OnMouseClick(bool left, bool press) { };
        // called when key is pressed (or released)
        virtual void OnKeyPress(int key, bool press) { };
        // called when UI action occured
        virtual void OnUIAction(uint32_t elementId, UIActionType actionId) { };
        // called when global action occured
        virtual void OnGlobalAction(GlobalActionIDs actionId, void* actionParam) { };

    protected:
        // protected constructor; instantiate only child classes
        StageTemplate(StageType type) : m_type(type) { };

    private:
        // stage type
        StageType m_type;
};

/*
 * Menu stage class
 */
class MenuStage : public StageTemplate
{
    public:
        MenuStage() : StageTemplate(STAGE_MENU) { };

        void OnEnter();
        void OnDraw();
        void OnUIAction(uint32_t elementId, UIActionType actionId);
        void OnGlobalAction(GlobalActionIDs actionId, void* actionParam);

    protected:
        //

    private:
        //
};

/*
 * Lobby stage class
 */
class LobbyStage : public StageTemplate
{
    public:
        LobbyStage() : StageTemplate(STAGE_LOBBY) { };

        void OnEnter();
        void OnDraw();
        void OnUIAction(uint32_t elementId, UIActionType actionId);
        void OnGlobalAction(GlobalActionIDs actionId, void* actionParam);

    protected:
        //

    private:
        // map of retrieved characters and their assignment to UI buttons
        std::map<uint32_t, uint32_t> m_characterListGUIDs;
};

/*
 * Connecting stage class
 */
class ConnectingStage : public StageTemplate
{
    public:
        ConnectingStage() : StageTemplate(STAGE_CONNECTING) { };

        void OnEnter();
        void OnDraw();
        void OnUIAction(uint32_t elementId, UIActionType actionId);
        void OnGlobalAction(GlobalActionIDs actionId, void* actionParam);

    protected:
        //

    private:
        //
};

class TextFieldWidget;
class InventoryWidget;

/*
 * Game stage class
 */
class GameStage : public StageTemplate
{
    public:
        GameStage() : StageTemplate(STAGE_GAME) { };

        void OnEnter();
        void OnLeave();
        void OnDraw();
        void OnMouseClick(bool left, bool press);
        void OnKeyPress(int key, bool press);
        void OnUIAction(uint32_t elementId, UIActionType actionId);
        void OnGlobalAction(GlobalActionIDs actionId, void* actionParam);

    protected:
        // opens chat (creates widget and sets focus)
        void OpenChat();
        // closes chat (destroys widget)
        void CloseChat();

        // opens inventory (creates widget)
        void OpenInventory();
        // closes inventory (destroys widget)
        void CloseInventory();

    private:
        // chat widget input field
        TextFieldWidget* m_chatWidget;
        // inventory widget
        InventoryWidget* m_inventoryWidget;
};

#endif

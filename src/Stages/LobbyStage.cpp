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
#include "Colors.h"
#include "Drawing.h"
#include "UI/LabelWidget.h"
#include "UI/ButtonWidget.h"
#include "UI/TextFieldWidget.h"
#include "UI/SplashMessageWidget.h"
#include "Stages.h"
#include "Gameplay.h"

#include "ResourceManager.h"

enum LobbyStageElementIDs
{
    ELEMENT_CHARACTER_BUTTON_START = 1000,
    ELEMENT_CHARACTER_BUTTON_END = 1009
};

void LobbyStage::OnEnter()
{
    // display splash message and wait for server response
    sDrawing->AddUIWidget(SplashMessageWidget::Create(SPLASH_TYPE_NORMAL, 500, FONT_MAIN, "Fetching character list...", false));
}

void LobbyStage::OnDraw()
{
    //
}

void LobbyStage::OnUIAction(uint32_t elementId, UIActionType actionId)
{
    // left click on UI widget
    if (actionId == UIACTION_MOUSE_CLICK_LEFT)
    {
        // ID is in character button range
        if (elementId >= ELEMENT_CHARACTER_BUTTON_START && elementId <= ELEMENT_CHARACTER_BUTTON_END)
        {
            // character exists
            if (m_characterListGUIDs.find(elementId) == m_characterListGUIDs.end())
                return;

            // sends enter world request
            sGameplay->EnterWorld(m_characterListGUIDs[elementId]);
        }
    }
}

void LobbyStage::OnGlobalAction(GlobalActionIDs actionId, void* actionParam)
{
    ButtonWidget* bw;
    LabelWidget* lw;

    switch (actionId)
    {
        case GA_CHARACTER_LIST_ACQUIRED:
        {
            // remove existing splash messages
            sDrawing->RemoveUIWidgetsOfType(UIWIDGET_SPLASHMESSAGE);
            // add title
            sDrawing->AddUIWidget(LabelWidget::Create(20, 20, FONT_MAIN, "Character list", BWCOLOR_BUZNABLUE));

            m_characterListGUIDs.clear();

            uint32_t i = 0;
            uint32_t currPos = 50;
            const uint32_t skipSize = 70;
            std::list<CharacterListRecord*> const& chars = sGameplay->GetCharacterList();

            // add button and labels for every character in list
            for (CharacterListRecord* rec : chars)
            {
                bw = ButtonWidget::Create(20, currPos, FONT_MAIN, " ");
                bw->SetDimensions(300, 60);
                bw->SetID(ELEMENT_CHARACTER_BUTTON_START + i);
                sDrawing->AddUIWidget(bw);

                m_characterListGUIDs[bw->GetID()] = rec->guid;

                lw = LabelWidget::Create(40, currPos + 13, FONT_MAIN, rec->name.c_str());
                lw->SetInteractive(false);
                sDrawing->AddUIWidget(lw);
                lw = LabelWidget::Create(40, currPos + 35, FONT_SMALLER, (std::string("Level ") + std::to_string(rec->level)).c_str());
                lw->SetInteractive(false);
                sDrawing->AddUIWidget(lw);

                currPos += skipSize;
                i++;
            }

            break;
        }
    }
}

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
#include "UI/InventoryWidget.h"
#include "UI/GamePanelWidget.h"
#include "Stages.h"
#include "Gameplay.h"
#include "Application.h"
#include "Unit.h"

void GameStage::OnEnter()
{
    m_chatWidget = nullptr;
    m_inventoryWidget = nullptr;
    m_gamePanel = GamePanelWidget::Create();
    sDrawing->SetDrawWorld(true);
}

void GameStage::OnLeave()
{
    sDrawing->SetDrawWorld(false);
}

void GameStage::OnDraw()
{
    //
}

void GameStage::OnMouseClick(bool left, bool press)
{
    if (left && !press && sGameplay->GetHoverObject())
        sGameplay->SendInteractionRequest(sGameplay->GetHoverObject());
}

void GameStage::OnKeyPress(int key, bool press)
{
    // several keys are handled only when no text input in progress
    if (!SDL_IsTextInputActive())
    {
        switch (key)
        {
            case SDLK_w:
                sGameplay->MovementKeyEvent(MOVE_UP, press);
                break;
            case SDLK_a:
                sGameplay->MovementKeyEvent(MOVE_LEFT, press);
                break;
            case SDLK_s:
                sGameplay->MovementKeyEvent(MOVE_DOWN, press);
                break;
            case SDLK_d:
                sGameplay->MovementKeyEvent(MOVE_RIGHT, press);
                break;
            case SDLK_i:
                if (press)
                {
                    if (!m_inventoryWidget)
                        OpenInventory();
                    else
                        CloseInventory();
                }
                break;
        }
    }

    switch (key)
    {
        case SDLK_RETURN:
            // on enter press, open chat or send chat message
            if (press)
            {
                if (!m_chatWidget)
                    OpenChat();
                else
                {
                    // send only when the text has at least one character
                    if (wcslen(m_chatWidget->GetText()) > 0)
                        sGameplay->SendChat(TALK_SAY, m_chatWidget->GetText());
                    CloseChat();
                }
            }
            break;
        case SDLK_ESCAPE:
            if (press)
                CloseChat();
            break;
    }
}

void GameStage::OnUIAction(uint32_t elementId, UIActionType actionId)
{
    switch (actionId)
    {
        case UIACTION_GAMEPANEL_INVENTORY:
            if (!m_inventoryWidget)
                OpenInventory();
            else
                CloseInventory();
            break;
        case UIACTION_GAMEPANEL_QUESTS:
            // TODO: implement quest board
            break;
    }
}

void GameStage::OnGlobalAction(GlobalActionIDs actionId, void* actionParam)
{
    //
}

void GameStage::OpenChat()
{
    if (m_chatWidget)
        return;

    // create chat widget
    m_chatWidget = TextFieldWidget::Create(0, 0, 300, FONT_CHAT, L"", BWCOLOR_CHATFIELDBG, BWCOLOR_BLACK);

    // position it to lower center part of screen
    TextFieldWidget* cap = m_chatWidget;
    m_chatWidget->SetPositionXLambda([cap](int32_t w, int32_t h) {
        return w / 2 - cap->GetWidth() / 2;
    });
    m_chatWidget->SetPositionYLambda([cap](int32_t w, int32_t h) {
        return h - cap->GetHeight() * 3;
    });

    sDrawing->AddUIWidget(m_chatWidget);
    // pass focus, so the user can immediatelly type
    sDrawing->SetFocusElement(m_chatWidget);
}

void GameStage::CloseChat()
{
    if (!m_chatWidget)
        return;

    sDrawing->RemoveUIWidget(m_chatWidget);

    delete m_chatWidget;
    m_chatWidget = nullptr;
}

void GameStage::OpenInventory()
{
    if (m_inventoryWidget)
        return;

    m_inventoryWidget = InventoryWidget::Create();

    sDrawing->AddUIWidget(m_inventoryWidget);
}

void GameStage::CloseInventory()
{
    if (!m_inventoryWidget)
        return;

    sDrawing->RemoveUIWidget(m_inventoryWidget);

    delete m_inventoryWidget;
    m_inventoryWidget = nullptr;
}

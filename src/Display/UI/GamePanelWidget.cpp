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
#include "Drawing.h"
#include "Colors.h"
#include "Gameplay.h"
#include "UI/GamePanelWidget.h"
#include "UI/UIEnums.h"
#include "ResourceManager.h"
#include "StorageManager.h"
#include "ItemCacheStorage.h"
#include "UI/StaticTooltipWidget.h"
#include "Application.h"

// defined game panel buttons
static const GamePanelButton gamePanelButtons[GAMEPANEL_BUTTON_COUNT] = {
    { 1,    UIACTION_GAMEPANEL_INVENTORY,      L"Inventáø (I)",    L"Otevøe / zavøe inventáø, kde jsou uchovány vìci" },
    { 2,    UIACTION_GAMEPANEL_QUESTS,         L"Úkoly (U)",       L"" },
};

GamePanelWidget::GamePanelWidget() : UIWidget(UIWIDGET_GAMEPANEL)
{
    m_buttonTooltip = StaticTooltipWidget::Create(L"", L"", 0);
    m_tooltipPos = -1;
}

GamePanelWidget::~GamePanelWidget()
{
    // with deletion, clear also tooltip widget created alongside with this widget
    if (m_tooltipPos >= 0)
    {
        m_tooltipPos = -1;
        sDrawing->RemoveUIWidget(m_buttonTooltip);
        sDrawing->SetCanvasRedrawFlag();
        delete m_buttonTooltip;
    }
}

GamePanelWidget* GamePanelWidget::Create()
{
    GamePanelWidget* gpw = new GamePanelWidget();

    gpw->DisableCanvasUpdate();

    gpw->SetWidth(GAMEPANEL_BOX_SIZE_PX);
    gpw->SetHeight(GAMEPANEL_BUTTON_COUNT * GAMEPANEL_BOX_SIZE_PX);

    // ... will be positioned to top right corner
    gpw->SetPositionXLambda([gpw](int32_t w, int32_t h){
        return w - gpw->GetWidth();
    });
    gpw->SetPositionY(10);

    gpw->EnableCanvasUpdate();
    gpw->UpdateCanvas();

    // This widget is automatically placed onto canvas
    sDrawing->AddUIWidget(gpw);

    return gpw;
}

void GamePanelWidget::OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY)
{
    // just signal specific event if needed
    if (left && press)
    {
        if (m_tooltipPos >= 0 && m_tooltipPos < GAMEPANEL_BUTTON_COUNT)
            sApplication->SignalUIEvent(m_id, gamePanelButtons[m_tooltipPos].actionType);
    }
}

void GamePanelWidget::OnMouseMove(uint32_t relativeX, uint32_t relativeY)
{
    // update tooltip according to current mouseover button
    int32_t detPos = relativeY / GAMEPANEL_BOX_SIZE_PX;
    if (detPos != m_tooltipPos && detPos >= 0 && detPos < GAMEPANEL_BUTTON_COUNT)
    {
        m_buttonTooltip->SetTitle(gamePanelButtons[detPos].tooltipTitle.c_str());
        m_buttonTooltip->SetText(gamePanelButtons[detPos].tooltipDescription.c_str());

        if (m_tooltipPos == -1)
            sDrawing->AddUIWidget(m_buttonTooltip);
        sDrawing->SetCanvasRedrawFlag();

        m_tooltipPos = detPos;
    }
}

void GamePanelWidget::OnMouseLeave()
{
    // clear tooltip when leaving widget area
    if (m_tooltipPos >= 0)
    {
        m_tooltipPos = -1;
        sDrawing->RemoveUIWidget(m_buttonTooltip);
        sDrawing->SetCanvasRedrawFlag();
    }
}

void GamePanelWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    UIWidget::UpdateCanvas();

    // destroy any previous prerendered texture
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    SDL_Renderer* renderer = sDrawing->GetRenderer();

    m_widgetCanvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GetWidth(), GetHeight());

    SDL_Texture* bgImg = sResourceManager->GetInternalImage(3);

    SDL_SetRenderTarget(renderer, m_widgetCanvas);

    // clear texture background, allow transparency
    SDL_SetTextureBlendMode(m_widgetCanvas, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_Rect dst;
    dst.w = GAMEPANEL_BOX_SIZE_PX;
    dst.h = GAMEPANEL_BOX_SIZE_PX;
    dst.x = 0;
    dst.y = 0;

    // draw underlaying boxes
    for (uint32_t i = 0; i < GAMEPANEL_BUTTON_COUNT; i++)
    {
        SDL_RenderCopy(renderer, bgImg, nullptr, &dst);
        dst.y += GAMEPANEL_BOX_SIZE_PX;
    }

    // draw panel icons
    dst.w = GAMEPANEL_ICON_SIZE_PX;
    dst.h = GAMEPANEL_ICON_SIZE_PX;
    dst.x = 8;
    dst.y = 8;
    SDL_Texture* fgImg;
    for (uint32_t i = 0; i < GAMEPANEL_BUTTON_COUNT; i++)
    {
        fgImg = sResourceManager->GetInternalImage(gamePanelButtons[i].internalImageId);
        if (fgImg)
            SDL_RenderCopy(renderer, fgImg, nullptr, &dst);

        dst.y += GAMEPANEL_BOX_SIZE_PX;
    }

    SDL_SetRenderTarget(renderer, nullptr);
}

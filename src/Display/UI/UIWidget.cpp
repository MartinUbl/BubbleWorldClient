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
#include "UI/UIEnums.h"
#include "UI/UIWidget.h"

UIWidget::UIWidget(UIWidgetType type)
{
    m_widgetCanvas = nullptr;
    m_positionXLambda = nullptr;
    m_positionYLambda = nullptr;
    m_interactive = true;

    m_forcedWidth = false;
    m_forcedHeight = false;

    SetWidgetType(type);
}

UIWidget::~UIWidget()
{
    //
}

void UIWidget::DisableCanvasUpdate()
{
    m_canvasUpdateEnabled = false;
}

void UIWidget::EnableCanvasUpdate()
{
    m_canvasUpdateEnabled = true;
}

UIWidget* UIWidget::SetID(uint32_t id)
{
    m_id = id;
    return this; // allow a little bit of fluency
}

void UIWidget::SetPositionX(int32_t x)
{
    m_dimensions.x = x;
}

void UIWidget::SetPositionY(int32_t y)
{
    m_dimensions.y = y;
}

void UIWidget::SetPosition(int32_t x, int32_t y)
{
    SetPositionX(x); SetPositionY(y);
}

void UIWidget::SetWidth(uint32_t w)
{
    m_dimensions.w = w;
    m_forcedWidth = true;
}

void UIWidget::SetHeight(uint32_t h)
{
    m_dimensions.h = h;
    m_forcedHeight = true;
}

void UIWidget::SetDimensions(uint32_t w, uint32_t h)
{
    SetWidth(w);
    SetHeight(h);
}

void UIWidget::SetPositionXLambda(std::function<int32_t(int32_t, int32_t)> func)
{
    m_positionXLambda = func;
}

void UIWidget::SetPositionYLambda(std::function<int32_t(int32_t, int32_t)> func)
{
    m_positionYLambda = func;
}

void UIWidget::SetWidthLambda(std::function<int32_t(int32_t, int32_t)> func)
{
    m_widthLambda = func;
}

void UIWidget::SetHeightLambda(std::function<int32_t(int32_t, int32_t)> func)
{
    m_heightLambda = func;
}

uint32_t UIWidget::GetID()
{
    return m_id;
}

UIWidgetType UIWidget::GetWidgetType()
{
    return m_widgetType;
}

int32_t UIWidget::GetPositionX()
{
    return m_dimensions.x;
}

int32_t UIWidget::GetPositionY()
{
    return m_dimensions.y;
}

uint32_t UIWidget::GetWidth()
{
    return m_dimensions.w;
}

uint32_t UIWidget::GetHeight()
{
    return m_dimensions.h;
}

void UIWidget::SetWidgetType(UIWidgetType type)
{
    m_widgetType = type;
}

bool UIWidget::IsCoordsInside(int32_t x, int32_t y)
{
    return (x > m_dimensions.x && x < m_dimensions.x + m_dimensions.w) && (y > m_dimensions.y && y < m_dimensions.y + m_dimensions.h);
}

bool UIWidget::IsInteractive()
{
    return m_interactive;
}

void UIWidget::SetInteractive(bool state)
{
    m_interactive = state;
}

void UIWidget::UpdateCanvas()
{
    // this method should be called at the end of every overriding UpdateCanvas method

    int32_t win_w, win_h;
    SDL_GetWindowSize(sDrawing->GetWindow(), &win_w, &win_h);

    if (m_widthLambda != nullptr)
        m_dimensions.w = m_widthLambda(win_w, win_h);
    if (m_heightLambda != nullptr)
        m_dimensions.h = m_heightLambda(win_w, win_h);
    if (m_positionXLambda != nullptr)
        m_dimensions.x = m_positionXLambda(win_w, win_h);
    if (m_positionYLambda != nullptr)
        m_dimensions.y = m_positionYLambda(win_w, win_h);

    sDrawing->SetCanvasRedrawFlag();
}

void UIWidget::OnDraw()
{
    // implicit behaviour: draw canvas to dest rectangle
    // child class overriding OnDraw should call UIWidget::OnDraw in its body
    if (m_widgetCanvas)
        sDrawing->DrawTexture(m_widgetCanvas, &m_dimensions);
}

void UIWidget::OnMouseEnter()
{
    // no implicit behavioour
}

void UIWidget::OnMouseLeave()
{
    // no implicit behavioour
}

void UIWidget::OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY)
{
    // no implicit behaviour
}

void UIWidget::OnMouseMove(uint32_t relativeX, uint32_t relativeY)
{
    // no implicit behaviour
}

bool UIWidget::OnKeyPress(int key, bool press)
{
    // no implicit behaviour
    return false;
}

void UIWidget::OnTextInput(wchar_t chr)
{
    // no implicit behaviour
}

void UIWidget::OnFocus()
{
    // no implicit behaviour
}

void UIWidget::OnBlur()
{
    // no implicit behaviour
}

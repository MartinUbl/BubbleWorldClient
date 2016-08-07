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
#include "UI/ButtonWidget.h"
#include "UI/UIEnums.h"

#include "Application.h"

ButtonWidget::ButtonWidget() : UIWidget(UIWIDGET_BUTTON)
{
    m_buttonState = BUTTON_STATE_NORMAL;

    for (int i = 0; i < MAX_BUTTON_STATE; i++)
        m_prerenderedStates[i] = nullptr;
}

ButtonWidget* ButtonWidget::Create(int32_t x, int32_t y, AppFonts fontId, const char* text, SDL_Color bgColor, SDL_Color textColor)
{
    ButtonWidget* bw = new ButtonWidget();

    // disable canvas update to avoid superfluous redraw between changes
    bw->DisableCanvasUpdate();

    // set parameters
    bw->SetPosition(x, y);
    bw->SetFontId(fontId);
    bw->SetText(text);
    bw->SetBackgroundColor(bgColor.r, bgColor.g, bgColor.b);
    bw->SetTextColor(textColor.r, textColor.g, textColor.b);
    bw->SetPaddingHorizontal(defaultButtonPaddingH);
    bw->SetPaddingVertical(defaultButtonPaddingV);

    // enable canvas update and perform redraw
    bw->EnableCanvasUpdate();
    bw->UpdateCanvas();

    return bw;
}

void ButtonWidget::SetText(const char* text)
{
    m_text = text;

    UpdateCanvas();
}

void ButtonWidget::SetTextColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_textColor.a = 0;
    m_textColor.r = r;
    m_textColor.g = g;
    m_textColor.b = b;

    UpdateCanvas();
}

void ButtonWidget::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_backgroundColor.a = 0;
    m_backgroundColor.r = r;
    m_backgroundColor.g = g;
    m_backgroundColor.b = b;

    UpdateCanvas();
}

void ButtonWidget::SetFontId(AppFonts fontId)
{
    m_fontId = fontId;

    UpdateCanvas();
}

void ButtonWidget::SetPaddingHorizontal(uint32_t paddingH)
{
    m_paddingH = paddingH;

    UpdateCanvas();
}

void ButtonWidget::SetPaddingVertical(uint32_t paddingV)
{
    m_paddingV = paddingV;

    UpdateCanvas();
}

void ButtonWidget::SetButtonState(ButtonState state)
{
    if (state >= MAX_BUTTON_STATE)
        return;

    m_buttonState = state;
    // switch drawn canvas to the one used for selected state
    m_widgetCanvas = m_prerenderedStates[state];

    sDrawing->SetCanvasRedrawFlag();
}

void ButtonWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    // destroy previously created textures
    for (int i = 0; i < MAX_BUTTON_STATE; i++)
    {
        if (m_prerenderedStates[i])
            SDL_DestroyTexture(m_prerenderedStates[i]);
    }

    // store text height for correct calculations
    int32_t textHeight = sDrawing->GetFontHeight(m_fontId, true);

    // prerender all states
    for (int i = 0; i < MAX_BUTTON_STATE; i++)
    {
        // create text surface and render text
        SDL_Surface* textsurf = sDrawing->RenderFont(m_fontId, m_text.c_str(), m_textColor);
        SDL_Rect textrect = { m_paddingH, m_paddingV, textsurf->w, textsurf->h };
        // adjust dimensions only when the size is not forced by method call
        if (!m_forcedWidth)
            m_dimensions.w = textsurf->w + 2 * m_paddingH;
        if (!m_forcedHeight)
            m_dimensions.h = textHeight + 2 * m_paddingV;

        // create backround surface
        SDL_Surface* destsurf = SDL_CreateRGBSurface(0, m_dimensions.w, m_dimensions.h, 32, 0, 0, 0, 0);
        // select background color for state
        SDL_Color const* bgclr = &m_backgroundColor;

        // TODO: customizable colors
        if (i == BUTTON_STATE_HOVER)
            bgclr = &defaultButtonBackColor_hover;
        else if (i == BUTTON_STATE_CLICKED)
            bgclr = &defaultButtonBackColor_clicked;

        // fill background with color
        SDL_FillRect(destsurf, nullptr, SDL_MapRGB(destsurf->format, bgclr->r, bgclr->g, bgclr->b));
        // apply text surface
        SDL_BlitSurface(textsurf, nullptr, destsurf, &textrect);

        // render button
        m_prerenderedStates[i] = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), destsurf);

        // cleanup
        SDL_FreeSurface(textsurf);
        SDL_FreeSurface(destsurf);
    }

    // currently used canvas is the one for "normal" state
    m_widgetCanvas = m_prerenderedStates[BUTTON_STATE_NORMAL];

    UIWidget::UpdateCanvas();
}

void ButtonWidget::OnMouseEnter()
{
    SetButtonState(BUTTON_STATE_HOVER);
}

void ButtonWidget::OnMouseLeave()
{
    SetButtonState(BUTTON_STATE_NORMAL);
}

void ButtonWidget::OnMouseClick(bool left, bool press)
{
    // on press, just acknowledge user he's clicking on button
    if (press)
        SetButtonState(BUTTON_STATE_CLICKED);
    else // on release, perform action
    {
        // check if we are still inside button widget
        if (IsCoordsInside(sApplication->GetMouseX(), sApplication->GetMouseY()))
        {
            // signal UI event to application
            sApplication->SignalUIEvent(m_id, left ? UIACTION_MOUSE_CLICK_LEFT : UIACTION_MOUSE_CLICK_RIGHT);
            // if there was some click function set, call it
            if (m_clickLambda != nullptr)
                m_clickLambda();
            // return state back to normal
            SetButtonState(BUTTON_STATE_HOVER);
        }
        else // if we left the button, return state to normal
            SetButtonState(BUTTON_STATE_NORMAL);
    }
}

void ButtonWidget::SetClickFunction(std::function<void(void)> fnc)
{
    m_clickLambda = fnc;
}

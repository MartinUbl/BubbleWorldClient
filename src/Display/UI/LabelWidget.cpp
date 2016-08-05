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
#include "UI/LabelWidget.h"
#include "UI/UIEnums.h"

LabelWidget::LabelWidget() : UIWidget(UIWIDGET_LABEL)
{
    //
}

LabelWidget* LabelWidget::Create(int32_t x, int32_t y, AppFonts fontId, const char* text, SDL_Color textColor)
{
    LabelWidget* lw = new LabelWidget();

    // disable canvas update after each change to avoid superfluous rendering
    lw->DisableCanvasUpdate();

    // set parameters
    lw->SetPosition(x, y);
    lw->SetFontId(fontId);
    lw->SetText(text);
    lw->SetTextColor(textColor.r, textColor.g, textColor.b);

    // enable canvas update and prerender output
    lw->EnableCanvasUpdate();
    lw->UpdateCanvas();

    return lw;
}

void LabelWidget::SetText(const char* text)
{
    m_text = text;

    UpdateCanvas();
}

void LabelWidget::SetTextColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_textColor.a = 0;
    m_textColor.r = r;
    m_textColor.g = g;
    m_textColor.b = b;

    UpdateCanvas();
}

void LabelWidget::SetFontId(AppFonts fontId)
{
    m_fontId = fontId;

    UpdateCanvas();
}

void LabelWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    // destroy any previously rendered textures
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    // render text on surface and cache size
    SDL_Surface* textsurf = sDrawing->RenderFont(m_fontId, m_text.c_str(), m_textColor);
    m_widgetCanvas = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), textsurf);
    sDrawing->GetTextureSize(m_widgetCanvas, &m_dimensions.w, &m_dimensions.h);

    SDL_FreeSurface(textsurf);

    UIWidget::UpdateCanvas();
}

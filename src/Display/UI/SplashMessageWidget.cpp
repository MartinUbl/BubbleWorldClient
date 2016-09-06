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
#include "UI/SplashMessageWidget.h"
#include "UI/UIEnums.h"

SplashMessageWidget::SplashMessageWidget() : UIWidget(UIWIDGET_SPLASHMESSAGE)
{
    //
}

SplashMessageWidget* SplashMessageWidget::Create(SplashMessageType type, int32_t baseWidth, AppFonts fontId, const wchar_t* text, bool cancellable, SDL_Color textColor)
{
    SplashMessageWidget* smw = new SplashMessageWidget();

    // disable canvas update to avoid superfluous rendering
    smw->DisableCanvasUpdate();

    // set horizontal and vertical centering using lambda functions
    smw->SetPositionXLambda([smw](int32_t win_w, int32_t win_h) {
        return win_w / 2 - smw->GetWidth() / 2;
    });
    smw->SetPositionYLambda([smw](int32_t win_w, int32_t win_h) {
        return win_h / 2 - smw->GetHeight() / 2;
    });

    // misc parameters
    smw->SetFontId(fontId);
    smw->SetText(text);
    smw->SetTextColor(textColor.r, textColor.g, textColor.b);

    // secure splash type range
    if (type > MAX_SPLASH_TYPE)
        type = SPLASH_TYPE_NORMAL;

    // color is determined by type
    smw->SetBackgroundColor(defaultSplashBackgroundColor[type].r, defaultSplashBackgroundColor[type].g, defaultSplashBackgroundColor[type].b);
    smw->SetBaseWidth(baseWidth);
    smw->SetPaddingHorizontal(defaultSplashPaddingH);
    smw->SetPaddingVertical(defaultSplashPaddingV);
    smw->SetType(type);
    smw->SetCancellable(cancellable);

    // enable canvas update and prerender widget
    smw->EnableCanvasUpdate();
    smw->UpdateCanvas();

    return smw;
}

void SplashMessageWidget::SetText(const wchar_t* text)
{
    m_text = text;

    UpdateCanvas();
}

void SplashMessageWidget::SetTextColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_textColor.a = 0;
    m_textColor.r = r;
    m_textColor.g = g;
    m_textColor.b = b;

    UpdateCanvas();
}

void SplashMessageWidget::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_backgroundColor.a = 0;
    m_backgroundColor.r = r;
    m_backgroundColor.g = g;
    m_backgroundColor.b = b;

    UpdateCanvas();
}

void SplashMessageWidget::SetFontId(AppFonts fontId)
{
    m_fontId = fontId;

    UpdateCanvas();
}

void SplashMessageWidget::SetBaseWidth(uint32_t width)
{
    m_baseWidth = width;

    UpdateCanvas();
}

void SplashMessageWidget::SetPaddingHorizontal(uint32_t paddingH)
{
    m_paddingH = paddingH;

    UpdateCanvas();
}

void SplashMessageWidget::SetPaddingVertical(uint32_t paddingV)
{
    m_paddingV = paddingV;

    UpdateCanvas();
}

void SplashMessageWidget::SetType(SplashMessageType type)
{
    if (type > MAX_SPLASH_TYPE)
        type = SPLASH_TYPE_NORMAL;

    m_type = type;

    // change color with type
    SetBackgroundColor(defaultSplashBackgroundColor[type].r, defaultSplashBackgroundColor[type].g, defaultSplashBackgroundColor[type].b);
}

void SplashMessageWidget::SetCancellable(bool state)
{
    m_cancellable = state;
}

void SplashMessageWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    // destroy any previous prerendered texture
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    // render text surface wrapped to base width
    SDL_Surface* textsurf = sDrawing->RenderFontWrappedUnicode(m_fontId, m_text.c_str(), m_baseWidth, m_textColor);
    // create background surface
    SDL_Surface* destsurf = SDL_CreateRGBSurface(0, m_baseWidth + 2 * m_paddingH, textsurf->h + 2 * m_paddingV, 32, 0, 0, 0, 0);
    // fill with background color
    SDL_FillRect(destsurf, nullptr, SDL_MapRGB(destsurf->format, m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b));
    SDL_Rect dstr = { m_paddingH, m_paddingV, textsurf->w, textsurf->h };

    // apply text surface
    SDL_BlitSurface(textsurf, nullptr, destsurf, &dstr);

    // create texture and cache size
    m_widgetCanvas = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), destsurf);
    sDrawing->GetTextureSize(m_widgetCanvas, &m_dimensions.w, &m_dimensions.h);

    // cleanup
    SDL_FreeSurface(textsurf);
    SDL_FreeSurface(destsurf);

    UIWidget::UpdateCanvas();
}

void SplashMessageWidget::OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY)
{
    // if not cancellable, do not do anything
    if (!m_cancellable)
        return;

    // remove self after click
    sDrawing->RemoveUIWidget(this);
    delete this;
}

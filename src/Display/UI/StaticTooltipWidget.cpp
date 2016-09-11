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
#include "UI/StaticTooltipWidget.h"
#include "UI/UIEnums.h"
#include "ResourceManager.h"

StaticTooltipWidget::StaticTooltipWidget() : UIWidget(UIWIDGET_STATICTOOLTIP)
{
    //
}

StaticTooltipWidget* StaticTooltipWidget::Create(const wchar_t* title, const wchar_t* text, uint32_t imageId, AppFonts titleFontId, AppFonts textFontId, SDL_Color titleColor, SDL_Color textColor)
{
    StaticTooltipWidget* stw = new StaticTooltipWidget();

    // disable canvas update to avoid superfluous rendering
    stw->DisableCanvasUpdate();

    // set horizontal and vertical position using lambda functions
    stw->SetPositionXLambda([stw](int32_t win_w, int32_t win_h) {
        return win_w / 2 - stw->GetWidth() / 2;
    });
    stw->SetPositionYLambda([stw](int32_t win_w, int32_t win_h) {
        return win_h - stw->GetHeight() - 10;
    });

    // misc parameters
    stw->SetTitleFontId(titleFontId);
    stw->SetTextFontId(textFontId);
    stw->SetTitle(title);
    stw->SetText(text);
    stw->SetTitleColor(titleColor.r, titleColor.g, titleColor.b);
    stw->SetTextColor(textColor.r, textColor.g, textColor.b);
    stw->SetImageId(imageId);

    // enable canvas update and prerender widget
    stw->EnableCanvasUpdate();
    stw->UpdateCanvas();

    return stw;
}

void StaticTooltipWidget::SetTitle(const wchar_t* text)
{
    if (text == nullptr || wcslen(text) == 0)
        m_title = L" ";
    else
        m_title = text;

    UpdateCanvas();
}

void StaticTooltipWidget::SetText(const wchar_t* text)
{
    if (text == nullptr)
        m_content = L"";
    else
        m_content = text;

    UpdateCanvas();
}

void StaticTooltipWidget::SetTitleColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_titleColor.a = 0;
    m_titleColor.r = r;
    m_titleColor.g = g;
    m_titleColor.b = b;

    UpdateCanvas();
}

void StaticTooltipWidget::SetTextColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_textColor.a = 0;
    m_textColor.r = r;
    m_textColor.g = g;
    m_textColor.b = b;

    UpdateCanvas();
}

void StaticTooltipWidget::SetTitleFontId(AppFonts fontId)
{
    m_titleFontId = fontId;

    UpdateCanvas();
}

void StaticTooltipWidget::SetTextFontId(AppFonts fontId)
{
    m_textFontId = fontId;

    UpdateCanvas();
}

void StaticTooltipWidget::SetImageId(uint32_t imageId)
{
    m_imageId = imageId;

    UpdateCanvas();
}

void StaticTooltipWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    // destroy any previous prerendered texture
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    SDL_Surface* tmpsurf;
    SDL_Renderer* renderer = sDrawing->GetRenderer();

    const uint32_t widgetWidth = 200;
    m_dimensions.w = 200;
    uint32_t textWidth = widgetWidth - defaultTooltipPadding * 2;
    if (m_imageId > 0)
        textWidth -= STATIC_TOOLTIP_ICON_SIZE_PX + defaultTooltipPadding;

    uint32_t totalHeight = defaultTooltipPadding * 2;

    // render text surface wrapped to width
    tmpsurf = sDrawing->RenderFontWrappedUnicode(m_titleFontId, m_title.c_str(), textWidth, m_titleColor);
    // store title height for future content text moving down
    uint32_t titleHeight = tmpsurf->h;
    totalHeight += tmpsurf->h;
    // create title texture and free the source surface
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, tmpsurf);
    SDL_FreeSurface(tmpsurf);

    // if the content text is defined, prepare it
    SDL_Texture* contTexture = nullptr;
    if (m_content.length() > 0)
    {
        tmpsurf = sDrawing->RenderFontWrappedUnicode(m_textFontId, m_content.c_str(), textWidth, m_textColor);
        totalHeight += tmpsurf->h + defaultTooltipPadding;
        contTexture = SDL_CreateTextureFromSurface(renderer, tmpsurf);
        SDL_FreeSurface(tmpsurf);
    }

    // create texture to be rendered onto
    m_widgetCanvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, widgetWidth, totalHeight);

    // render target scope
    {
        RenderTargetGuard rguard(m_widgetCanvas);

        SDL_SetRenderDrawColor(renderer, defaultTooltipBackgroundColor.r, defaultTooltipBackgroundColor.g, defaultTooltipBackgroundColor.b, 0xFF);
        SDL_RenderClear(renderer);

        SDL_Rect dstrect;

        // if the image is requested to be drawn, draw it and move text a bit to the right
        uint32_t textBase = defaultTooltipPadding;
        if (m_imageId > 0)
        {
            dstrect.w = STATIC_TOOLTIP_ICON_SIZE_PX;
            dstrect.h = STATIC_TOOLTIP_ICON_SIZE_PX;
            dstrect.x = defaultTooltipPadding;
            dstrect.y = (totalHeight - STATIC_TOOLTIP_ICON_SIZE_PX) / 2;

            textBase += STATIC_TOOLTIP_ICON_SIZE_PX + defaultTooltipPadding;
            SDL_Texture* img = sResourceManager->GetImage(m_imageId);
            if (img)
                SDL_RenderCopy(renderer, img, nullptr, &dstrect);
        }

        // render title text
        SDL_QueryTexture(titleTexture, nullptr, nullptr, &dstrect.w, &dstrect.h);
        dstrect.x = textBase;
        dstrect.y = defaultTooltipPadding;
        SDL_RenderCopy(renderer, titleTexture, nullptr, &dstrect);

        // if the content text texture is defined, put it onto canvas
        if (contTexture)
        {
            SDL_QueryTexture(contTexture, nullptr, nullptr, &dstrect.w, &dstrect.h);
            dstrect.x = textBase;
            dstrect.y = titleHeight + defaultTooltipPadding * 2;
            SDL_RenderCopy(renderer, contTexture, nullptr, &dstrect);
        }
    }

    // don't forget to cache height
    m_dimensions.h = totalHeight;

    // cleanup
    SDL_DestroyTexture(titleTexture);
    if (contTexture)
        SDL_DestroyTexture(contTexture);

    UIWidget::UpdateCanvas();
}

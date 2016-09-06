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

#ifndef BW_STATICTOOLTIPWIDGET_H
#define BW_STATICTOOLTIPWIDGET_H

#include "UIWidget.h"

// defined color for tooltip background
static const SDL_Color defaultTooltipBackgroundColor = { 96, 96, 96 };
// default tooltip padding
static const uint32_t defaultTooltipPadding = 10;
// default content text color
static const SDL_Color defaultContentTextColor = { 192, 192, 192 };

// static tooltip image size, if requested (in pixels; square)
#define STATIC_TOOLTIP_ICON_SIZE_PX 32

/*
 * Widget representing static tooltip widget - widget at fixed place with title, text and icon
 */
class StaticTooltipWidget : public UIWidget
{
    public:
        // static factory method for creating static toolitip widgets
        static StaticTooltipWidget* Create(const wchar_t* title, const wchar_t* text, uint32_t imageId = 0, AppFonts titleFontId = FONT_TOOLTIP_TITLE, AppFonts textFontId = FONT_TOOLTIP_TEXT, SDL_Color titleColor = defaultTextColor, SDL_Color textColor = defaultContentTextColor);

        // updates prerendered texture
        void UpdateCanvas();

        // sets title text
        void SetTitle(const wchar_t* text);
        // sets content text
        void SetText(const wchar_t* text);
        // sets title text color
        void SetTitleColor(uint8_t r, uint8_t g, uint8_t b);
        // sets content text color
        void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
        // sets used font (for title)
        void SetTitleFontId(AppFonts fontId);
        // sets used font (for text content)
        void SetTextFontId(AppFonts fontId);
        // sets image ID used
        void SetImageId(uint32_t imageId);

    protected:
        // protected constructor; use factory method Create for instantiation
        StaticTooltipWidget();

    private:
        // title text drawn
        std::wstring m_title;
        // content text drawn
        std::wstring m_content;
        // title color
        SDL_Color m_titleColor;
        // text color
        SDL_Color m_textColor;
        // font used for title
        AppFonts m_titleFontId;
        // font used for content text
        AppFonts m_textFontId;
        // texture used
        uint32_t m_imageId;
};

#endif

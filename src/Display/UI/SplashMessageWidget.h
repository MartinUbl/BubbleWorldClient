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

#ifndef BW_SPLASHMESSAGEWIDGET_H
#define BW_SPLASHMESSAGEWIDGET_H

#include "UIWidget.h"

// type of splash message - defines color
enum SplashMessageType
{
    SPLASH_TYPE_NORMAL,
    SPLASH_TYPE_SUCCESS,
    SPLASH_TYPE_ERROR,
    SPLASH_TYPE_WARNING,
    MAX_SPLASH_TYPE
};

// defined color for splash backgrounds
static const SDL_Color defaultSplashBackgroundColor[MAX_SPLASH_TYPE] = {
    {  75,  75,  75 },
    {  75, 125,  75 },
    { 125,  75,  75 },
    { 125, 125,  75 }
};
// default splash message widget horizontal padding
static const uint32_t defaultSplashPaddingH = 20;
// default splash message widget vertical padding
static const uint32_t defaultSplashPaddingV = 20;

/*
 * Widget representing splash message element - text on background with basic interactivity
 */
class SplashMessageWidget : public UIWidget
{
    public:
        // static factory method for creating splash message widgets
        static SplashMessageWidget* Create(SplashMessageType type, int32_t baseWidth, AppFonts fontId, const char* text, bool cancellable = true, SDL_Color textColor = defaultTextColor);

        // updates prerendered texture
        void UpdateCanvas();

        void OnMouseClick(bool left, bool press);

        // sets foreground text
        void SetText(const char* text);
        // sets foreground text color
        void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
        // sets background color
        void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
        // sets used font
        void SetFontId(AppFonts fontId);
        // sets base width for widget (excludes padding); text then wraps to this size
        void SetBaseWidth(uint32_t width);
        // sets message type
        void SetType(SplashMessageType type);
        // sets cancellable state (whether the message could be closed by click or not)
        void SetCancellable(bool state);
        // sets horizontal padding
        void SetPaddingHorizontal(uint32_t paddingH);
        // sets vertical padding
        void SetPaddingVertical(uint32_t paddingV);

    protected:
        // protected constructor; use factory method Create for instantiation
        SplashMessageWidget();

    private:
        // text drawn
        std::string m_text;
        // text color
        SDL_Color m_textColor;
        // background color (determined by type)
        SDL_Color m_backgroundColor;
        // type of message
        SplashMessageType m_type;
        // is message cancellable by clicking?
        bool m_cancellable;
        // base width (excludes padding)
        uint32_t m_baseWidth;
        // horizontal padding
        uint32_t m_paddingH;
        // vertical padding
        uint32_t m_paddingV;
        // font used
        AppFonts m_fontId;
};

#endif

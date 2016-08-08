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

#ifndef BW_BUTTONWIDGET_H
#define BW_BUTTONWIDGET_H

#include "UIWidget.h"

// default color for button background
static const SDL_Color defaultButtonBackColor = { 90, 90, 90 };
// default color for button background when mouse is over
static const SDL_Color defaultButtonBackColor_hover = { 150, 150, 150 };
// default color for button when clicking
static const SDL_Color defaultButtonBackColor_clicked = { 50, 50, 50 };
// default buttom horizontal padding
static const uint32_t defaultButtonPaddingH = 20;
// default button vertical padding
static const uint32_t defaultButtonPaddingV = 8;

// button state
enum ButtonState
{
    BUTTON_STATE_NORMAL = 0,        // no special state
    BUTTON_STATE_HOVER = 1,         // mouse over
    BUTTON_STATE_CLICKED = 2,       // mouse is clicking
    MAX_BUTTON_STATE
};

/*
 * Widget which represents standard button
 */
class ButtonWidget : public UIWidget
{
    public:
        // static factory method for creating Button widget
        static ButtonWidget* Create(int32_t x, int32_t y, AppFonts fontId, const wchar_t* text, SDL_Color bgColor = defaultButtonBackColor, SDL_Color textColor = defaultTextColor);

        // updates cached canvas after changes made
        void UpdateCanvas();

        void OnMouseEnter();
        void OnMouseLeave();
        void OnMouseClick(bool left, bool press);

        // stores click action (does not bypass UI event broadcast)
        void SetClickFunction(std::function<void(void)> fnc);

        // sets foreground text
        void SetText(const wchar_t* text);
        // sets foreground text color
        void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
        // sets background color
        void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
        // sets font used
        void SetFontId(AppFonts fontId);

        // sets horizontal padding
        void SetPaddingHorizontal(uint32_t paddingH);
        // sets vertical padding
        void SetPaddingVertical(uint32_t paddingV);

        // sets button state
        void SetButtonState(ButtonState state);

    protected:
        // hidden constructor; use factory method Create for instantiation
        ButtonWidget();

    private:
        // foreground text
        std::wstring m_text;
        // text color used
        SDL_Color m_textColor;
        // background color used (normal state)
        SDL_Color m_backgroundColor;
        // font used
        AppFonts m_fontId;
        // horizontal padding
        uint32_t m_paddingH;
        // vertical padding
        uint32_t m_paddingV;

        // current button state
        ButtonState m_buttonState;
        // prerendered textures of these buttons
        SDL_Texture* m_prerenderedStates[MAX_BUTTON_STATE];
        // click lambda function
        std::function<void(void)> m_clickLambda;
};

#endif

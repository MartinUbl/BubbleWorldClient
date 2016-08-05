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

#ifndef BW_TEXTFIELDWIDGET_H
#define BW_TEXTFIELDWIDGET_H

#include "UIWidget.h"

// default textfield background color
static const SDL_Color defaultTextFieldBackgroundColor = { 50, 50, 50 };
// default textfield background color when mouse is over
static const SDL_Color defaultTextFieldBackgroundColor_hover = { 50, 50, 50 };
// default textfield background color when the focus is on
static const SDL_Color defaultTextFieldBackgroundColor_focus = { 120, 120, 120 };

// default textfield horizontal padding
static const uint32_t defaultTextFieldPaddingH = 10;
// default textfield vertical padding
static const uint32_t defaultTextFieldPaddingV = 8;

// text field states
enum TextFieldState
{
    TEXTFIELD_STATE_NORMAL = 0,         // normal, no special state
    TEXTFIELD_STATE_HOVER = 1,          // mouse is over
    TEXTFIELD_STATE_FOCUS = 2,          // the focus is in (we can type in)
    MAX_TEXTFIELD_STATE
};

/*
 * Widget representing text input field element
 */
class TextFieldWidget : public UIWidget
{
    public:
        // static factory method for creating text field widget
        static TextFieldWidget* Create(int32_t x, int32_t y, int32_t baseWidth, AppFonts fontId, const char* text, SDL_Color bgColor = defaultTextFieldBackgroundColor, SDL_Color textColor = defaultTextColor);

        // updates prerendered textures
        void UpdateCanvas();

        void OnFocus();
        void OnBlur();
        bool OnKeyPress(int key, bool press);

        // sets foreground text
        void SetText(const char* text);
        // sets foreground text color
        void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
        // sets background color
        void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
        // sets font used
        void SetFontId(AppFonts fontId);
        // sets base width of whole field
        void SetBaseWidth(int32_t width);
        // sets horizontal padding
        void SetPaddingHorizontal(uint32_t paddingH);
        // sets vertical padding
        void SetPaddingVertical(uint32_t paddingV);
        // sets text field state
        void SetTextFieldState(TextFieldState state);
        // sets character for masking (i.e. '*' for passwords, ..)
        void SetMaskCharacter(char chr);
        // retrieves text entered by user
        const char* GetText();

    protected:
        // protected constructor; use factory method Create for instantiation
        TextFieldWidget();

    private:
        // text entered by user
        std::string m_text;
        // character used for masking real text
        char m_maskCharacter;
        // text color used
        SDL_Color m_textColor;
        // background color (normal state)
        SDL_Color m_backgroundColor;
        // font used
        AppFonts m_fontId;
        // base width (excluding padding)
        int32_t m_baseWidth;
        // horizontal padding
        uint32_t m_paddingH;
        // vertical padding
        uint32_t m_paddingV;
        // textfield state
        TextFieldState m_state;
        // cached prerendered state
        SDL_Texture* m_prerenderedStates[MAX_TEXTFIELD_STATE];
};

#endif

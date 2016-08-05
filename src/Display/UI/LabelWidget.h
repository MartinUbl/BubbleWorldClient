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

#ifndef BW_LABELWIDGET_H
#define BW_LABELWIDGET_H

#include "UIWidget.h"

/*
 * Widget containing just text label
 */
class LabelWidget : public UIWidget
{
    public:
        // static factory method for creating label widget
        static LabelWidget* Create(int32_t x, int32_t y, AppFonts fontId, const char* text, SDL_Color textColor = defaultTextColor);

        // updates canvas for drawing
        void UpdateCanvas();

        // sets foreground text
        void SetText(const char* text);
        // sets foreground text color
        void SetTextColor(uint8_t r, uint8_t g, uint8_t b);
        // sets font used
        void SetFontId(AppFonts fontId);

    protected:
        // protected constructor; use factory method Create for instantiation
        LabelWidget();

    private:
        // text drawn
        std::string m_text;
        // text color used
        SDL_Color m_textColor;
        // font used
        AppFonts m_fontId;
};

#endif

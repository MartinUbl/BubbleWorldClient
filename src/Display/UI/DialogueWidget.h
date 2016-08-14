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

#ifndef BW_DIALOGUEWIDGET_H
#define BW_DIALOGUEWIDGET_H

#include "UIWidget.h"

// defined color for dialogue backgrounds
static const SDL_Color defaultDialogueBackgroundColor = { 128, 192, 128 };
// default dialogue widget horizontal padding
static const uint32_t dialoguePaddingH = 20;
// default dialogue widget vertical padding
static const uint32_t dialoguePaddingV = 20;
// default dialogue widget button spacing
static const uint32_t dialogueButtonSpacing = 5;

class ButtonWidget;

/*
 * Structure containing dialogue decision
 */
struct DialogueWidgetDecision
{
    DialogueWidgetDecision(uint32_t _id, ButtonWidget* _button) : id(_id), button(_button) {};

    // decision ID (sent to server)
    uint32_t id;
    // button widget assigned
    ButtonWidget* button;
};

/*
 * Widget representing dialogue element - box with dialogue decisions
 */
class DialogueWidget : public UIWidget
{
    public:
        // static factory method for creating splash message widgets
        static DialogueWidget* Create(const wchar_t* text);

        // updates prerendered texture
        void UpdateCanvas();

        // sets foreground text
        void SetText(const wchar_t* text);
        // adds decision to decision list
        void AddDecision(uint32_t id, const wchar_t* text);
        // removes all decisions
        void ClearDecisions();

    protected:
        // protected constructor; use factory method Create for instantiation
        DialogueWidget();

        // height of header text
        uint32_t m_headerTextHeight;

    private:
        // text drawn
        std::wstring m_text;
        // vector of buttons for this dialogue widget (position on screen is determined by position in this vector)
        std::vector<DialogueWidgetDecision> m_decisionButtons;
};

#endif

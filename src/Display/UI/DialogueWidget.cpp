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
#include "Colors.h"
#include "Gameplay.h"
#include "UI/ButtonWidget.h"
#include "UI/DialogueWidget.h"
#include "UI/UIEnums.h"

DialogueWidget::DialogueWidget() : UIWidget(UIWIDGET_DIALOGUE)
{
    m_headerTextHeight = 0;
}

DialogueWidget* DialogueWidget::Create(const wchar_t* text)
{
    DialogueWidget* dw = new DialogueWidget();

    dw->DisableCanvasUpdate();

    // the decision dialog would be 1/4 of window width wide
    dw->SetWidthLambda([dw](int32_t w, int32_t h) {
        return (w / 4);
    });
    // ... and half of window height tall
    dw->SetHeightLambda([dw](int32_t w, int32_t h) {
        return (h / 2);
    });

    // ... will be positioned somewhere to the right in window
    dw->SetPositionXLambda([dw](int32_t w, int32_t h){
        return w - (w / 4) - (w / 8);
    });
    // ... and to the top part
    dw->SetPositionYLambda([dw](int32_t w, int32_t h){
        return (h / 4);
    });

    dw->SetText(text);

    dw->EnableCanvasUpdate();
    dw->UpdateCanvas();

    // This widget is automatically placed onto canvas due to Z-index determined by drawing order
    sDrawing->AddUIWidget(dw);

    return dw;
}

void DialogueWidget::SetText(const wchar_t* text)
{
    m_text = text;

    UpdateCanvas();

    // the text size may change, this will recalculate button positions
    for (DialogueWidgetDecision& dec : m_decisionButtons)
        dec.button->UpdateCanvas();
}

void DialogueWidget::AddDecision(uint32_t id, const wchar_t* text)
{
    uint32_t position = (uint32_t)m_decisionButtons.size();

    ButtonWidget* bw = ButtonWidget::Create(0, 0, FONT_SMALLER, text);
    // buttons are positioned to the dialogue rectangle, moved by padding
    bw->SetPositionXLambda([](int32_t w, int32_t h) {
        return w - (w / 4) - (w / 8) + dialoguePaddingH;
    });
    // Y position is calculated using position, padding, button spacing and title text height
    bw->SetPositionYLambda([this, position, bw](int32_t w, int32_t h) {
        return (h / 4) + dialoguePaddingH * 2 + m_headerTextHeight + position * (bw->GetHeight() + dialogueButtonSpacing);
    });
    // the click function should signal gameplay class about decision selection
    bw->SetClickFunction([id](){
        sGameplay->SignalDialogueDecision(id);
    });

    // add decision button to our vector
    m_decisionButtons.push_back(DialogueWidgetDecision(id, bw));

    // automatically draw the button
    sDrawing->AddUIWidget(bw);

    UpdateCanvas();
}

void DialogueWidget::ClearDecisions()
{
    // clear all buttons
    for (DialogueWidgetDecision& dec : m_decisionButtons)
    {
        sDrawing->RemoveUIWidget(dec.button);
        delete dec.button;
    }

    m_decisionButtons.clear();

    UpdateCanvas();
}

void DialogueWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    // at first, call parent method - it will resize the widget for us, so we can calculate with real dimensions
    UIWidget::UpdateCanvas();

    // destroy any previous prerendered texture
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    // inner width
    int32_t baseWidth = m_dimensions.w - 2 * dialoguePaddingH;

    // render text surface wrapped to dialog width
    SDL_Surface* textsurf = sDrawing->RenderFontWrappedUnicode(FONT_SMALLER, m_text.c_str(), baseWidth, BWCOLOR_BLACK);
    m_headerTextHeight = textsurf->h;

    // calculate how much space do we need for decision buttons
    uint32_t buttonSumH;

    if (m_decisionButtons.size() > 0)
    {
        buttonSumH = dialoguePaddingV;
        for (DialogueWidgetDecision& dec : m_decisionButtons)
            buttonSumH += dec.button->GetHeight() + dialogueButtonSpacing;
        buttonSumH -= dialogueButtonSpacing;
    }
    else
        buttonSumH = 0;

    // create background surface
    SDL_Surface* destsurf = SDL_CreateRGBSurface(0, baseWidth + 2 * dialoguePaddingH, textsurf->h + 2 * dialoguePaddingV + buttonSumH, 32, 0, 0, 0, 0);
    // fill with background color
    SDL_FillRect(destsurf, nullptr, SDL_MapRGB(destsurf->format, defaultDialogueBackgroundColor.r, defaultDialogueBackgroundColor.g, defaultDialogueBackgroundColor.b));
    SDL_Rect dstr = { dialoguePaddingH, dialoguePaddingV, textsurf->w, textsurf->h };

    // apply text surface
    SDL_BlitSurface(textsurf, nullptr, destsurf, &dstr);

    // create texture and cache size
    m_widgetCanvas = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), destsurf);
    sDrawing->GetTextureSize(m_widgetCanvas, &m_dimensions.w, &m_dimensions.h);

    // cleanup
    SDL_FreeSurface(textsurf);
    SDL_FreeSurface(destsurf);
}

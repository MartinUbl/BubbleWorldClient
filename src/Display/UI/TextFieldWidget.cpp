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
#include "UI/TextFieldWidget.h"
#include "UI/UIEnums.h"

#include "Application.h"

TextFieldWidget::TextFieldWidget() : UIWidget(UIWIDGET_TEXTFIELD)
{
    m_state = TEXTFIELD_STATE_NORMAL;
    m_maskCharacter = 0;

    for (int i = 0; i < MAX_TEXTFIELD_STATE; i++)
        m_prerenderedStates[i] = nullptr;
}

TextFieldWidget* TextFieldWidget::Create(int32_t x, int32_t y, int32_t baseWidth, AppFonts fontId, const char* text, SDL_Color bgColor, SDL_Color textColor)
{
    TextFieldWidget* tw = new TextFieldWidget();

    // disable canvas update to avoid superfluous rendering
    tw->DisableCanvasUpdate();

    // set parameters
    tw->SetPosition(x, y);
    tw->SetFontId(fontId);
    tw->SetText(text);
    tw->SetBackgroundColor(bgColor.r, bgColor.g, bgColor.b);
    tw->SetTextColor(textColor.r, textColor.g, textColor.b);
    tw->SetPaddingHorizontal(defaultTextFieldPaddingH);
    tw->SetPaddingVertical(defaultTextFieldPaddingV);
    tw->SetBaseWidth(baseWidth);
    tw->SetTextFieldState(TEXTFIELD_STATE_NORMAL);

    // enable canvas update and prerender
    tw->EnableCanvasUpdate();
    tw->UpdateCanvas();

    return tw;
}

void TextFieldWidget::SetText(const char* text)
{
    m_text = text;

    UpdateCanvas();
}

void TextFieldWidget::SetTextColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_textColor.a = 0;
    m_textColor.r = r;
    m_textColor.g = g;
    m_textColor.b = b;

    UpdateCanvas();
}

void TextFieldWidget::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
    m_backgroundColor.a = 0;
    m_backgroundColor.r = r;
    m_backgroundColor.g = g;
    m_backgroundColor.b = b;

    UpdateCanvas();
}

void TextFieldWidget::SetFontId(AppFonts fontId)
{
    m_fontId = fontId;

    UpdateCanvas();
}

void TextFieldWidget::SetPaddingHorizontal(uint32_t paddingH)
{
    m_paddingH = paddingH;

    UpdateCanvas();
}

void TextFieldWidget::SetPaddingVertical(uint32_t paddingV)
{
    m_paddingV = paddingV;

    UpdateCanvas();
}

void TextFieldWidget::SetBaseWidth(int32_t width)
{
    m_baseWidth = width;

    UpdateCanvas();
}

const char* TextFieldWidget::GetText()
{
    return m_text.c_str();
}

void TextFieldWidget::SetTextFieldState(TextFieldState state)
{
    if (m_state == state)
        return;

    m_state = state;
    // change also appearance with state
    m_widgetCanvas = m_prerenderedStates[state];

    sDrawing->SetCanvasRedrawFlag();
}

void TextFieldWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    m_widgetCanvas = nullptr;

    // destroy previously created textures
    for (int i = 0; i < MAX_TEXTFIELD_STATE; i++)
    {
        if (m_prerenderedStates[i])
            SDL_DestroyTexture(m_prerenderedStates[i]);
    }

    // build text used for displaying
    std::string displayText;
    // when no masking character, display the text itself
    if (m_maskCharacter == 0)
        displayText = (m_text.size() == 0) ? " " : m_text.c_str();
    else
    {
        // mask the text using exact same number of masking characters
        displayText = (m_text.size() == 0) ? " " : "";
        for (size_t i = 0; i < m_text.size(); i++)
            displayText += m_maskCharacter;
    }

    // store font height for calculations
    int32_t textHeight = sDrawing->GetFontHeight(m_fontId);

    // prerender all states
    for (int i = 0; i < MAX_TEXTFIELD_STATE; i++)
    {
        // create text surface by rendering display text
        SDL_Surface* textsurf = sDrawing->RenderFont(m_fontId, (i == TEXTFIELD_STATE_FOCUS) ? (displayText + " |").c_str() : displayText.c_str(), m_textColor);
        SDL_Rect textrect = { m_paddingH, m_paddingV, num_min(textsurf->w, m_baseWidth), textsurf->h };
        // clip maximum of *basewidth* pixels from text
        SDL_Rect srcrect = { textsurf->w > m_baseWidth ? textsurf->w - m_baseWidth : 0, 0, num_min(textsurf->w, m_baseWidth), textsurf->h };
        m_dimensions.w = m_baseWidth + 2 * m_paddingH;
        m_dimensions.h = textHeight + 2 * m_paddingV;

        // prepare background surface
        SDL_Surface* destsurf = SDL_CreateRGBSurface(0, m_dimensions.w, m_dimensions.h, 32, 0, 0, 0, 0);
        // select background color
        SDL_Color const* bgclr = &m_backgroundColor;

        // TODO: customizable colors
        if (i == TEXTFIELD_STATE_HOVER)
            bgclr = &defaultTextFieldBackgroundColor_hover;
        else if (i == TEXTFIELD_STATE_FOCUS)
            bgclr = &defaultTextFieldBackgroundColor_focus;

        // fills background with color
        SDL_FillRect(destsurf, nullptr, SDL_MapRGB(destsurf->format, bgclr->r, bgclr->g, bgclr->b));
        // apply text surface
        SDL_BlitSurface(textsurf, &srcrect, destsurf, &textrect);

        // convert to texture
        m_prerenderedStates[i] = SDL_CreateTextureFromSurface(sDrawing->GetRenderer(), destsurf);

        // cleanup
        SDL_FreeSurface(textsurf);
        SDL_FreeSurface(destsurf);
    }

    // select "current" state
    m_widgetCanvas = m_prerenderedStates[m_state];

    UIWidget::UpdateCanvas();
}

void TextFieldWidget::OnFocus()
{
    SetTextFieldState(TEXTFIELD_STATE_FOCUS);
}

void TextFieldWidget::OnBlur()
{
    SetTextFieldState(TEXTFIELD_STATE_NORMAL);
}

bool TextFieldWidget::OnKeyPress(int key, bool press)
{
    // process only "on press" events
    if (press)
    {
        // transform letters according to shift/capslock state
        // TODO: rework this system to unicode, utilizing SDL unicode support; this will then be no longer needed
        if ((sApplication->GetShiftState() && !sApplication->GetCapsLockState()) || sApplication->GetCapsLockState())
        {
            if (key >= 'a' && key <= 'z')
                key = key - 'a' + 'A';
        }

        // consider this range "printable" characters
        if (key >= SDLK_SPACE && key < SDLK_DELETE)
        {
            SetText((m_text + (char)key).c_str());
        }
        // backspace - delete last letter if the length is sufficient
        else if (key == SDLK_BACKSPACE)
        {
            if (m_text.size() > 0)
                SetText(m_text.substr(0, m_text.size() - 1).c_str());
        }
        // keys we don't capture (to allow i.e. closing chat action, etc.)
        else if (key == SDLK_ESCAPE || (key >= SDLK_F1 && key <= SDLK_F12) || key == SDLK_RETURN)
            return false;
    }

    return true;
}

void TextFieldWidget::SetMaskCharacter(char chr)
{
    m_maskCharacter = chr;
}

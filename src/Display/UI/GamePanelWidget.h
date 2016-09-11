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

#ifndef BW_GAMEPANELWIDGET_H
#define BW_GAMEPANELWIDGET_H

#include "UIWidget.h"

// size in pixels of gamepanel box
#define GAMEPANEL_BOX_SIZE_PX 48
// size in pixels of gamepanel icon
#define GAMEPANEL_ICON_SIZE_PX 32
// how many buttons there are on the panel
#define GAMEPANEL_BUTTON_COUNT 2

/*
 * Structure containing static data about game panel buttons
 */
struct GamePanelButton
{
    // internal image ID
    uint32_t internalImageId;
    // assigned UI action
    UIActionType actionType;
    // short tooltip
    std::wstring tooltipTitle;
    // description
    std::wstring tooltipDescription;
};

class StaticTooltipWidget;

/*
 * Widget representing game panel widget
 */
class GamePanelWidget : public UIWidget
{
    public:
        virtual ~GamePanelWidget();

        // static factory method for creating game panel widget
        static GamePanelWidget* Create();

        void OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY);
        void OnMouseMove(uint32_t relativeX, uint32_t relativeY);
        void OnMouseLeave();

        // updates prerendered texture
        void UpdateCanvas();

    protected:
        // protected constructor; use factory method Create for instantiation
        GamePanelWidget();

    private:
        // tooltip widget created alongside this widget
        StaticTooltipWidget* m_buttonTooltip;
        // current button position for which the tooltip is created
        int32_t m_tooltipPos;
};

#endif

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

#ifndef BW_INVENTORYWIDGET_H
#define BW_INVENTORYWIDGET_H

#include "UIWidget.h"

// defined color for inventory backgrounds
static const SDL_Color defaultInventoryBackgroundColor = { 96, 96, 96 };
// defined color for inventory "taken slot"
static const SDL_Color defaultInventoryTakenSlotColor = { 128, 128, 128 };
// defined color for inventory "empty slot"
static const SDL_Color defaultInventoryEmptySlotColor = { 192, 192, 192 };
// default arrow color for navigating through inventory
static const SDL_Color defaultInventoryArrowColor = { 192, 192, 192 };
// default arrow color when cannot navigate further in inventory
static const SDL_Color defaultInventoryArrowDisabledColor = { 128, 128, 128 };
// default inventory widget horizontal padding
static const uint32_t inventoryPaddingH = 20;
// default inventory widget vertical padding
static const uint32_t inventoryPaddingV = 20;
// default inventory widget items spacing
static const uint32_t inventoryItemSpacing = 5;

// total slots per inventory widget row
#define INVENTORY_SLOT_PER_ROW 5
// total slots per inventory widget column
#define INVENTORY_SLOT_PER_COLUMN 5
// inventory item size (in pixels; square)
#define INVENTORY_ITEM_SIZE_PX 32

class StaticTooltipWidget;

/*
 * Widget representing inventory widget
 */
class InventoryWidget : public UIWidget
{
    public:
        virtual ~InventoryWidget();

        // static factory method for creating splash message widgets
        static InventoryWidget* Create();

        void OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY);
        void OnMouseMove(uint32_t relativeX, uint32_t relativeY);
        void OnMouseLeave();

        // updates prerendered texture
        void UpdateCanvas();

    protected:
        // protected constructor; use factory method Create for instantiation
        InventoryWidget();

    private:
        // how many rows have we scrolled down using arrows?
        uint8_t m_rowOffset;
        // tooltip widget created alongside inventory widget
        StaticTooltipWidget* m_itemTooltip;
        // current slot, which is used to draw tooltip - never empty slot; -1 for "none"
        int32_t m_tooltipSlot;
        // always set to valid slot, when the mouse is over - may be empty; -1 for "none"
        int32_t m_hoverSlot;
        // slot we are dragging item from; -1 for "none"
        int32_t m_dragSourceSlot;
};

#endif

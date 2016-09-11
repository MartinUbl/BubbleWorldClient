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
#include "UI/InventoryWidget.h"
#include "UI/UIEnums.h"
#include "ResourceManager.h"
#include "StorageManager.h"
#include "ItemCacheStorage.h"
#include "UI/StaticTooltipWidget.h"

InventoryWidget::InventoryWidget() : UIWidget(UIWIDGET_INVENTORY)
{
    m_rowOffset = 0;
    m_itemTooltip = StaticTooltipWidget::Create(L"", L"", 0);
    m_tooltipSlot = -1;
    m_dragSourceSlot = -1;
    m_hoverSlot = -1;
}

InventoryWidget::~InventoryWidget()
{
    // with deletion, clear also tooltip widget created alongside with this widget
    if (m_tooltipSlot >= 0)
    {
        m_tooltipSlot = -1;
        sDrawing->RemoveUIWidget(m_itemTooltip);
        sDrawing->SetCanvasRedrawFlag();
        delete m_itemTooltip;
    }

    if (m_dragSourceSlot >= 0)
    {
        m_dragSourceSlot = -1;
        sDrawing->RestoreCursor();
        sDrawing->SetCanvasRedrawFlag();
    }
}

InventoryWidget* InventoryWidget::Create()
{
    InventoryWidget* iw = new InventoryWidget();

    iw->DisableCanvasUpdate();

    // 5 items in a row
    iw->SetWidth(inventoryPaddingH * 2 + INVENTORY_SLOT_PER_ROW * INVENTORY_ITEM_SIZE_PX + (INVENTORY_SLOT_PER_ROW - 1) * inventoryItemSpacing);
    // 5 items in a column
    iw->SetHeight(inventoryPaddingV * 2 + INVENTORY_SLOT_PER_COLUMN * INVENTORY_ITEM_SIZE_PX + (INVENTORY_SLOT_PER_COLUMN - 1) * inventoryItemSpacing);

    // ... will be positioned somewhere to the right in window
    iw->SetPositionXLambda([iw](int32_t w, int32_t h){
        return w - iw->GetWidth() - 10;
    });
    // ... and to the bottom
    iw->SetPositionYLambda([iw](int32_t w, int32_t h){
        return h - iw->GetHeight() - 10;
    });

    iw->EnableCanvasUpdate();
    iw->UpdateCanvas();

    // This widget is automatically placed onto canvas due to Z-index determined by drawing order
    sDrawing->AddUIWidget(iw);

    return iw;
}

void InventoryWidget::OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY)
{
    if (left && press)
    {
        if (relativeX >= GetWidth() / 2 - inventoryPaddingV + 5 && relativeX <= GetWidth() / 2 + inventoryPaddingV - 5)
        {
            // "up" arrow
            if (relativeY >= 5 && relativeY <= inventoryPaddingV - 5)
            {
                if (m_rowOffset > 0)
                {
                    m_rowOffset--;
                    UpdateCanvas();
                    sDrawing->SetCanvasRedrawFlag();
                }
            }
            // "down" arrow
            else if (relativeY >= GetHeight() - inventoryPaddingV + 5 && relativeY <= GetHeight() - 5)
            {
                if (m_rowOffset + INVENTORY_SLOT_PER_COLUMN < CHARACTER_INVENTORY_SLOTS / INVENTORY_SLOT_PER_ROW)
                {
                    m_rowOffset++;
                    UpdateCanvas();
                    sDrawing->SetCanvasRedrawFlag();
                }
            }
        }

        // if there is some item to be dragged (tooltipSlot is the sign of it)
        if (m_tooltipSlot >= 0 && m_dragSourceSlot == -1)
        {
            InventoryItem* item = sGameplay->GetInventorySlot(m_tooltipSlot);
            if (item)
            {
                ItemCacheEntry* ic = sGameplay->GetItemCacheEntry(item->id);
                if (ic)
                {
                    // set drag source and set cursor
                    m_dragSourceSlot = m_tooltipSlot;
                    sDrawing->SetCursorDragImage(ic->imageId);
                    sDrawing->SetCanvasRedrawFlag();
                }
            }
        }
    }

    // mouse release
    if (left && !press)
    {
        // if we dragged some item
        if (m_dragSourceSlot >= 0)
        {
            // if the source and destination slots are valid, swap it
            if (m_hoverSlot >= 0 && m_dragSourceSlot != m_hoverSlot)
                sGameplay->SendSwapInventorySlots(m_dragSourceSlot, m_hoverSlot);

            // restore cursor back to normal
            sDrawing->RestoreCursor();
            sDrawing->SetCanvasRedrawFlag();
            m_dragSourceSlot = -1;
        }
    }
}

void InventoryWidget::OnMouseMove(uint32_t relativeX, uint32_t relativeY)
{
    // we are interested just in mouse movement inside active element area (excluding padding)
    if (relativeX > inventoryPaddingH && relativeX < GetWidth() - inventoryPaddingH &&
        relativeY > inventoryPaddingV && relativeY < GetHeight() - inventoryPaddingV)
    {
        int32_t detSlot = -1;
        int32_t relPos;

        // get size of box of one item (include padding)
        int32_t perItemSize = (GetWidth() - inventoryPaddingH * 2) / INVENTORY_SLOT_PER_ROW;

        // if the offset within box exceeds the icon (is in padding part), the mouse is not in slot
        if ((relativeX - inventoryPaddingH) % perItemSize < INVENTORY_ITEM_SIZE_PX)
        {
            // calculate X-index of slot
            relPos = (relativeX - inventoryPaddingH) / perItemSize;

            // repeat the same conditional madness with the second coordinate
            perItemSize = (GetHeight() - inventoryPaddingV * 2) / INVENTORY_SLOT_PER_ROW;
            // if everything OK, calculate final slot offset; don't forget to consider rowOffset set by moving in inventory by arrows
            if ((relativeY - inventoryPaddingV) % perItemSize < INVENTORY_ITEM_SIZE_PX)
                detSlot = (m_rowOffset + ((relativeY - inventoryPaddingV) / perItemSize)) * INVENTORY_SLOT_PER_ROW + relPos;
        }

        // if we changed the slot (from "none" to valid, or between two different valid), update tooltip
        if (detSlot >= 0 && m_tooltipSlot != detSlot)
        {
            InventoryItem* item = sGameplay->GetInventorySlot(detSlot);
            if (item)
            {
                ItemCacheEntry* ic = sGameplay->GetItemCacheEntry(item->id);
                if (ic)
                {
                    // update contents
                    m_itemTooltip->SetTitle(ic->name.c_str());
                    m_itemTooltip->SetText(ic->description.c_str());
                    m_itemTooltip->SetImageId(ic->imageId);

                    // if not displayed yet, display
                    if (m_tooltipSlot == -1)
                        sDrawing->AddUIWidget(m_itemTooltip);

                    sDrawing->SetCanvasRedrawFlag();
                    m_tooltipSlot = detSlot;
                }
            }
        }
        // or if we moved mouse from valid slot to "none", clear tooltip
        else if (detSlot == -1 && m_tooltipSlot >= 0)
        {
            m_tooltipSlot = -1;
            sDrawing->RemoveUIWidget(m_itemTooltip);
            sDrawing->SetCanvasRedrawFlag();
        }

        m_hoverSlot = detSlot;
    }
    else
    {
        // if not within the slot area at all, clear tooltip
        if (m_tooltipSlot >= 0)
        {
            m_tooltipSlot = -1;
            sDrawing->RemoveUIWidget(m_itemTooltip);
            sDrawing->SetCanvasRedrawFlag();
        }

        m_hoverSlot = -1;
    }
}

void InventoryWidget::OnMouseLeave()
{
    // clear tooltip when leaving widget area
    if (m_tooltipSlot >= 0)
    {
        m_tooltipSlot = -1;
        sDrawing->RemoveUIWidget(m_itemTooltip);
        sDrawing->SetCanvasRedrawFlag();
    }

    // stop "drag and drop" when leaving widget area
    if (m_dragSourceSlot >= 0)
    {
        m_dragSourceSlot = -1;
        sDrawing->RestoreCursor();
        sDrawing->SetCanvasRedrawFlag();
    }

    m_hoverSlot = -1;
}

void InventoryWidget::UpdateCanvas()
{
    if (!m_canvasUpdateEnabled)
        return;

    UIWidget::UpdateCanvas();

    // destroy any previous prerendered texture
    if (m_widgetCanvas)
        SDL_DestroyTexture(m_widgetCanvas);

    SDL_Renderer* renderer = sDrawing->GetRenderer();

    SDL_Surface* emptysurf = SDL_CreateRGBSurface(0, INVENTORY_ITEM_SIZE_PX, INVENTORY_ITEM_SIZE_PX, 32, 0, 0, 0, 0);
    // fill with "empty" color
    SDL_FillRect(emptysurf, nullptr, SDL_MapRGB(emptysurf->format, defaultInventoryEmptySlotColor.r, defaultInventoryEmptySlotColor.g, defaultInventoryEmptySlotColor.b));
    SDL_Texture* emptyslottexture = SDL_CreateTextureFromSurface(renderer, emptysurf);
    SDL_FreeSurface(emptysurf);

    SDL_Surface* takensurf = SDL_CreateRGBSurface(0, INVENTORY_ITEM_SIZE_PX, INVENTORY_ITEM_SIZE_PX, 32, 0, 0, 0, 0);
    // fill with "taken" color
    SDL_FillRect(takensurf, nullptr, SDL_MapRGB(takensurf->format, defaultInventoryTakenSlotColor.r, defaultInventoryTakenSlotColor.g, defaultInventoryTakenSlotColor.b));
    SDL_Texture* takenslottexture = SDL_CreateTextureFromSurface(renderer, takensurf);
    SDL_FreeSurface(takensurf);

    // create texture to be used as background
    m_widgetCanvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GetWidth(), GetHeight());

    SDL_SetRenderTarget(renderer, m_widgetCanvas);

    // clear with bg color
    SDL_SetRenderDrawColor(renderer, defaultInventoryBackgroundColor.r, defaultInventoryBackgroundColor.g, defaultInventoryBackgroundColor.b, 0xFF);
    SDL_RenderClear(renderer);

    // prepare destination rectangle for drawing slots
    SDL_Rect dst;
    SDL_Rect countDest;
    dst.x = inventoryPaddingH;
    dst.y = inventoryPaddingV;
    dst.w = INVENTORY_ITEM_SIZE_PX;
    dst.h = INVENTORY_ITEM_SIZE_PX;
    InventoryItem* item;

    // prepare arrow color; if we cannot move higher, change color to "disabled"
    SDL_Color arrCol = defaultInventoryArrowColor;
    if (m_rowOffset == 0)
        arrCol = defaultInventoryArrowDisabledColor;

    // draw arrow "up"
    filledTrigonRGBA(renderer,
        GetWidth() / 2, 5,
        GetWidth() / 2 - inventoryPaddingV + 5, inventoryPaddingV - 5,
        GetWidth() / 2 + inventoryPaddingV - 5, inventoryPaddingV - 5,
        arrCol.r, arrCol.g, arrCol.b, 0xFF);

    // do the same for arrow "down"

    if (m_rowOffset + INVENTORY_SLOT_PER_COLUMN >= CHARACTER_INVENTORY_SLOTS / INVENTORY_SLOT_PER_ROW)
        arrCol = defaultInventoryArrowDisabledColor;
    else
        arrCol = defaultInventoryArrowColor;

    filledTrigonRGBA(renderer,
        GetWidth() / 2, GetHeight() - 5,
        GetWidth() / 2 - inventoryPaddingV + 5, GetHeight() - inventoryPaddingV + 5,
        GetWidth() / 2 + inventoryPaddingV - 5, GetHeight() - inventoryPaddingV + 5,
        arrCol.r, arrCol.g, arrCol.b, 0xFF);

    // draw slots; consider row offset set by moving using arrows
    for (uint32_t i = m_rowOffset; i < (uint32_t)(m_rowOffset + INVENTORY_SLOT_PER_ROW); i++)
    {
        for (uint32_t j = 0; j < INVENTORY_SLOT_PER_COLUMN; j++)
        {
            // if the item is not present, draw blank slot
            item = sGameplay->GetInventorySlot(i * INVENTORY_SLOT_PER_ROW + j);
            if (!item)
                SDL_RenderCopy(renderer, emptyslottexture, nullptr, &dst);
            else // otherwise draw "taken" slot and image of item, if available
            {
                SDL_RenderCopy(renderer, takenslottexture, nullptr, &dst);

                ItemCacheEntry* ic = sGameplay->GetItemCacheEntry(item->id);
                // render just if the item cache entry and its image is available at all
                if (ic && ic->imageId != 0)
                {
                    SDL_Texture* itemtxt = sResourceManager->GetImage(ic->imageId);
                    if (itemtxt)
                        SDL_RenderCopy(renderer, itemtxt, nullptr, &dst);

                    // if the item has more than one item in stack, draw count
                    if (item->stackCount > 1)
                    {
                        SDL_Surface* countsurf;

                        // at first render background layer

                        // outline guard scope
                        {
                            FontOutlineGuard fg(FONT_TOOLTIP_TEXT, 2);
                            countsurf = sDrawing->RenderFontUnicode(FONT_TOOLTIP_TEXT, std::to_wstring(item->stackCount).c_str(), BWCOLOR_BLACK);
                        }

                        countDest.w = countsurf->w;
                        countDest.h = countsurf->h;
                        SDL_Texture* counttxt = SDL_CreateTextureFromSurface(renderer, countsurf);
                        SDL_FreeSurface(countsurf);

                        countDest.x = dst.x + INVENTORY_ITEM_SIZE_PX - countDest.w - 1;
                        countDest.y = dst.y + INVENTORY_ITEM_SIZE_PX - countDest.h - 1;

                        SDL_RenderCopy(renderer, counttxt, nullptr, &countDest);
                        SDL_DestroyTexture(counttxt);

                        // render foreground layer
                        countsurf = sDrawing->RenderFontUnicode(FONT_TOOLTIP_TEXT, std::to_wstring(item->stackCount).c_str(), BWCOLOR_WHITE);

                        countDest.w = countsurf->w;
                        countDest.h = countsurf->h;
                        counttxt = SDL_CreateTextureFromSurface(renderer, countsurf);
                        SDL_FreeSurface(countsurf);

                        // this is approximate, the real position change might be different
                        countDest.x += 2;
                        countDest.y += 2;

                        SDL_RenderCopy(renderer, counttxt, nullptr, &countDest);
                        SDL_DestroyTexture(counttxt);
                    }
                }
            }

            // move on X axis
            dst.x += INVENTORY_ITEM_SIZE_PX + inventoryItemSpacing;
        }

        // reset X axis and move on Y axis
        dst.y += INVENTORY_ITEM_SIZE_PX + inventoryItemSpacing;
        dst.x = inventoryPaddingH;
    }

    SDL_SetRenderTarget(renderer, nullptr);

    SDL_RenderPresent(renderer);

    // cleanup
    SDL_DestroyTexture(emptyslottexture);
    SDL_DestroyTexture(takenslottexture);
}

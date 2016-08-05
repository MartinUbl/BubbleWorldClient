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

#ifndef BW_UIENUMS_H
#define BW_UIENUMS_H

// UI widget type
enum UIWidgetType
{
    UIWIDGET_BUTTON,
    UIWIDGET_LABEL,
    UIWIDGET_SPLASHMESSAGE,
    UIWIDGET_TEXTFIELD
};

// UI action type (used for propagating through application)
enum UIActionType
{
    UIACTION_MOUSE_CLICK_LEFT = 0,
    UIACTION_MOUSE_CLICK_RIGHT = 1,
    UIACTION_TEXT_CHANGED = 2
};

#endif

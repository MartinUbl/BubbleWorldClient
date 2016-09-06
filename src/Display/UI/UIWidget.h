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

#ifndef BW_UIWIDGET_H
#define BW_UIWIDGET_H

#include "UI/UIEnums.h"

/*
 * Base class for all UI widgets
 */
class UIWidget
{
    public:
        virtual ~UIWidget();

        // disables updating canvas after each change
        void DisableCanvasUpdate();
        // enables updating canvas after each change
        void EnableCanvasUpdate();

        // sets widget ID (optional parameter)
        UIWidget* SetID(uint32_t id);
        // sets absolute X position (is overriden by SetPositionXLambda)
        void SetPositionX(int32_t x);
        // sets absolute Y position (is overriden by SetPositionYLambda)
        void SetPositionY(int32_t y);
        // sets X;Y position (overriden by SetPositionXLambda, SetPositionYLambda)
        void SetPosition(int32_t x, int32_t y);
        // forces width (overrides automatic scaling in several widgets) (overriden by SetWidthLambda)
        void SetWidth(uint32_t w);
        // forces height (overrides automatic scaling in several widgets) (overriden by SetHeightLambda)
        void SetHeight(uint32_t h);
        // sets width and height (overrides automatic scaling in several widgets) (overriden by SetWidthLambda, SetHeightLambda)
        void SetDimensions(uint32_t w, uint32_t h);

        // sets lambda function for determining X position coordinate (args: windowWidth, windowHeight)
        void SetPositionXLambda(std::function<int32_t(int32_t, int32_t)> func);
        // sets lambda function for determining Y position coordinate (args: windowWidth, windowHeight)
        void SetPositionYLambda(std::function<int32_t(int32_t, int32_t)> func);
        // sets lambda function for determining widget width (args: windowWidth, windowHeight)
        void SetWidthLambda(std::function<int32_t(int32_t, int32_t)> func);
        // sets lambda function for determining widget height (args: windowWidth, windowHeight)
        void SetHeightLambda(std::function<int32_t(int32_t, int32_t)> func);

        // retrieves widget ID
        uint32_t GetID();
        // retrieves widget type
        UIWidgetType GetWidgetType();
        // retrieves X position coordinate
        int32_t GetPositionX();
        // retrieves Y position coordinate
        int32_t GetPositionY();
        // retrieves width
        uint32_t GetWidth();
        // retrieves height
        uint32_t GetHeight();
        // determines if specified coordinates are inside widget's rectangle
        virtual bool IsCoordsInside(int32_t x, int32_t y);
        // determines whether the widget should be interactive for user input (mouse, ..)
        bool IsInteractive();
        // sets interactivity flag - is the widget supposed to react to user input (mouse, ..)?
        void SetInteractive(bool state);

        // update widget canvas (prerender texture)
        virtual void UpdateCanvas();
        // when the widget is drawn; most of operations performs base class itself, derived class should not draw anything complicated here; use caching
        virtual void OnDraw();
        // when mouse enters the widget area (hover)
        virtual void OnMouseEnter();
        // when mouse leaves the widget area
        virtual void OnMouseLeave();
        // when user clicks into widget area
        virtual void OnMouseClick(bool left, bool press, uint32_t relativeX, uint32_t relativeY);
        // when user moves the mouse over widget area
        virtual void OnMouseMove(uint32_t relativeX, uint32_t relativeY);
        // when the widget has focus and user presses key; returns true when captured, false to propagate key press further
        virtual bool OnKeyPress(int key, bool press);
        // when the text is typed
        virtual void OnTextInput(wchar_t chr);
        // when the widget gains focus
        virtual void OnFocus();
        // when the widget loses focus
        virtual void OnBlur();

    protected:
        // protected constructor; use derived class factory methods to instantiate widgets
        UIWidget(UIWidgetType type);

        // sets widget type
        void SetWidgetType(UIWidgetType type);

        // widget ID
        uint32_t m_id;
        // should update canvas after change?
        bool m_canvasUpdateEnabled;
        // type of widget
        UIWidgetType m_widgetType;
        // cached prerendered texture to be drawn
        SDL_Texture* m_widgetCanvas;
        // position, width and height
        SDL_Rect m_dimensions;
        // is interactive to user input?
        bool m_interactive;
        // is width forced by SetWidth?
        bool m_forcedWidth;
        // is height forced by SetHeight?
        bool m_forcedHeight;
        // lambda function used to calculate X position coordinate
        std::function<int32_t(int32_t, int32_t)> m_positionXLambda; 
        // lambda function used to calculate Y position coordinate
        std::function<int32_t(int32_t, int32_t)> m_positionYLambda;
        // lambda function used to calculate width
        std::function<int32_t(int32_t, int32_t)> m_widthLambda;
        // lambda function used to calculate height
        std::function<int32_t(int32_t, int32_t)> m_heightLambda;

    private:
        //
};

#endif

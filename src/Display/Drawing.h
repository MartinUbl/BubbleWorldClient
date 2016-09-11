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

#ifndef BW_DRAWING_H
#define BW_DRAWING_H

#include "Singleton.h"
#include "UI/UIEnums.h"

// initial window width, may be overriden by config setting
#define DEF_WINDOW_WIDTH 1024
// initial window height, may be overriden by config setting
#define DEF_WINDOW_HEIGHT 768

// X size of map field (pixels)
#define MAP_FIELD_PX_SIZE_X 32
// Y size of map field (pixels)
#define MAP_FIELD_PX_SIZE_Y 32

// width of chat frame (chat message history)
#define CHAT_MSG_FRAME_WIDTH 300
// height of chat frame (chat message history)
#define CHAT_MSG_FRAME_HEIGHT 150
// maximum count of chat messages stored in history
#define CHAT_MSG_MAX_COUNT 15

// enumeration of application font IDs
enum AppFonts
{
    FONT_MAIN = 0,
    FONT_BIGGER = 1,
    FONT_CAPTION = 2,
    FONT_SMALLER = 3,
    FONT_NAME_TITLE = 4,
    FONT_CHAT = 5,
    FONT_TOOLTIP_TITLE = 6,
    FONT_TOOLTIP_TEXT = 7,
    MAX_FONT
};

/*
 * Structure used to initialize application fonts
 */
struct FontParam
{
    // font name or filename
    const char* fontName;
    // size in px
    uint16_t size;
};

// default text color used when no color specified
static const SDL_Color defaultTextColor = { 255, 255, 255 };

// enumeration of application cursor IDs
enum AppMouseCursors
{
    MOUSE_CURSOR_NORMAL = 0,
    MOUSE_CURSOR_TALK = 1,
    MOUSE_CURSOR_TEMP = 2,
    MAX_MOUSE_CURSOR
};

class UIWidget;
class MouseCursor;

/*
 * Singleton class maintaining everything related to drawing
 */
class Drawing
{
    friend class Singleton<Drawing>;
    public:
        ~Drawing();

        // retrieves application window
        SDL_Window* GetWindow() { return m_window; };
        // retrieves SDL renderer
        SDL_Renderer* GetRenderer() { return m_renderer; };

        // initializes drawing; returns true on success, false on failure
        bool Init();
        // update drawing (redraw screen if needed, determine framerate, ..)
        void Update();

        // adds UI widget to draw list
        void AddUIWidget(UIWidget* widget);
        // removes UI widget from draw list
        void RemoveUIWidget(UIWidget* widget);
        // remove all UI widgets of type from draw list
        void RemoveUIWidgetsOfType(UIWidgetType type);
        // destroys all UI elements
        void DestroyUI();
        // retrieves UI widget by its ID (has to be previously set by SetID on widget instance)
        UIWidget* FindUIWidgetById(uint32_t id);
        // does some UI widget have hover now?
        bool HasUIWidgetHover();
        // called when mouse moved
        void OnMouseMove(int32_t x, int32_t y);
        // called when mouse click event fired
        void OnMouseClick(bool left, bool press, uint32_t x, uint32_t y);
        // called when key press has been fired; returns true if captured, false to propagate further
        bool OnKeyPress(int key, bool press);
        // called when in text input and the text is typed
        void OnTextInput(char* orig);
        // sets focus to element
        void SetFocusElement(UIWidget* widget);
        // should the world be drawn?
        void SetDrawWorld(bool state);
        // draw specified portion of texture to target coordinates
        void DrawTexture(SDL_Texture* texture, SDL_Rect* source, SDL_Rect* target);
        // draw whole texture to target coordinates
        void DrawTexture(SDL_Texture* texture, SDL_Rect* target);
        // renders font on surface
        SDL_Surface* RenderFont(AppFonts fontId, const char* message, SDL_Color color = defaultTextColor);
        // renders font on surface using unicode methods
        SDL_Surface* RenderFontUnicode(AppFonts fontId, const wchar_t* message, SDL_Color color = defaultTextColor);
        // renders font on surface and wraps words to specified width
        SDL_Surface* RenderFontWrapped(AppFonts fontId, const char* message, uint32_t width, SDL_Color color = defaultTextColor);
        // renders font on surface and wraps words to specified width
        SDL_Surface* RenderFontWrappedUnicode(AppFonts fontId, const wchar_t* message, uint32_t width, SDL_Color color = defaultTextColor);
        // sets font outline; do not forget to set it back after being done!!
        void SetFontOutline(AppFonts fontId, uint32_t outlinePx);
        // retrieves font height from font metrics
        uint16_t GetFontHeight(AppFonts fontId, bool useAscent = false);
        // queries SDL_Texture for its dimensions
        void GetTextureSize(SDL_Texture* texture, int* w, int* h);
        // sets current mouse cursor type
        void SetMouseCursor(AppMouseCursors type);
        // retrieves current mouse cursor type
        AppMouseCursors GetCurrentMouseCursor();
        // adds image under cursor
        void SetCursorDragImage(uint32_t imageId);
        // removes image from under the cursor and restores original cursor
        void RestoreCursor();
        // sets flag to redraw whole canvas
        void SetCanvasRedrawFlag();
        // sets flag to redraw UI elements ("rerender")
        void SetUIRedrawFlag();

    protected:
        // protected singleton constructor
        Drawing();

        // draw everything connected with gameplay (world, objects, ..)
        void DrawWorld();
        // redraws all UI elements (prerender to their textures), due to i.e. new image load, etc.
        void RerenderUIElements();

    private:
        // should the canvas be redrawn?
        bool m_canvasRedraw;
        // should the UI elements be redrawn?
        bool m_uiRedraw;
        // should the world be drawn?
        bool m_drawWorld;
        // main window
        SDL_Window* m_window;
        // main renderer
        SDL_Renderer* m_renderer;

        // mutex for widget list
        std::recursive_mutex m_widgetListMtx;
        // widget list to be drawn
        std::list<UIWidget*> m_widgetList;
        // element which is currently under mouse cursor
        UIWidget* m_hoverElement;
        // element which holds focus
        UIWidget* m_focusElement;
        // cached application fonts
        TTF_Font* m_fonts[MAX_FONT];
        // cached application mouse cursors
        MouseCursor* m_mouseCursors[MAX_MOUSE_CURSOR];
        // current mouse cursor used
        AppMouseCursors m_currentMouseCursor;

        // main window width
        int32_t m_windowWidth;
        // main window height
        int32_t m_windowHeight;
        // last update (used for FPS counter)
        uint32_t m_lastUpdate;
        // FPS counter (update count per second)
        uint32_t m_fpsCounter;
};

#define sDrawing Singleton<Drawing>::getInstance()

/*
 * Structure serving as scope guard for "critical" temporary change of font outline
 */
struct FontOutlineGuard
{
    // constructor sets font outline
    FontOutlineGuard(AppFonts fontId, uint32_t outlinePx)
    {
        m_fontId = fontId;
        sDrawing->SetFontOutline(m_fontId, outlinePx);
    }
    // destructor resets font outline back to normal
    ~FontOutlineGuard()
    {
        sDrawing->SetFontOutline(m_fontId, 0);
    }

    // font that has its outline changed
    AppFonts m_fontId;
};

#endif

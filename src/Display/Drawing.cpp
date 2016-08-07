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
#include "UI/UIEnums.h"
#include "UI/UIWidget.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "Gameplay.h"
#include "ImageStorage.h"
#include "ResourceManager.h"
#include "MouseCursor.h"

// application font settings
static const FontParam appFontParams[MAX_FONT] = {
    { "SpecialElite.ttf", 16 },                 // FONT_MAIN
    { "Gretoon.ttf", 28 },                      // FONT_BIGGER
    { "Gretoon.ttf", 20 },                      // FONT_CAPTION
    { "SpecialElite.ttf", 12 },                 // FONT_SMALLER
    { "SpecialElite.ttf", 14 },                 // FONT_NAME_TITLE
    { "GFSNeohellenic.ttf", 15 }                // FONT_CHAT
};

// application cursor filenames
static const char* appCursorFilenames[MAX_MOUSE_CURSOR] = {
    "main.png",
    "talk.png"
};

Drawing::Drawing()
{
    m_canvasRedraw = true;
    m_hoverElement = nullptr;
    m_focusElement = nullptr;
    m_drawWorld = false;
    m_currentMouseCursor = MAX_MOUSE_CURSOR;
}

Drawing::~Drawing()
{
    //
}

bool Drawing::Init()
{
    // turn on basic multisampling
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
    // turn on acceleration
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // TODO: load window dimensions from config or something like that
    m_windowWidth = DEF_WINDOW_WIDTH;
    m_windowHeight = DEF_WINDOW_HEIGHT;

    // create main window using default parameters and OpenGL context
    m_window = SDL_CreateWindow("BubbleWorld",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_windowWidth, m_windowHeight,
        SDL_WINDOW_OPENGL);

    if (!m_window)
    {
        sLog->Error("Could not initialize SDL window");
        return false;
    }

    // create main renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);

    if (!m_renderer)
    {
        sLog->Error("Could not create SDL renderer");
        return false;
    }

    // turn on v-sync
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_GL_SetSwapInterval(-1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // initialize font library
    if (TTF_Init() == -1)
    {
        sLog->Error("Could not initialize SDL TTF library");
        return false;
    }

    // initialize application fonts
    for (int i = 0; i < MAX_FONT; i++)
    {
        m_fonts[i] = TTF_OpenFont(appFontParams[i].fontName, appFontParams[i].size);
        // if no installed font with this name found, use file
        if (!m_fonts[i])
        {
            std::string path = FONTS_DATA_DIR + std::string(appFontParams[i].fontName);
            m_fonts[i] = TTF_OpenFont(path.c_str(), appFontParams[i].size);
        }

        if (!m_fonts[i])
            sLog->Error("Could not load font ID %i", i);
    }

    // initialize application mouse cursors
    for (int i = 0; i < MAX_MOUSE_CURSOR; i++)
    {
        std::string path = CURSORS_DATA_DIR + std::string(appCursorFilenames[i]);
        m_mouseCursors[i] = MouseCursor::LoadFromFile(path.c_str());

        if (!m_mouseCursors[i])
            sLog->Error("Could not load mouse cursor ID %i", i);
    }

    // set normal mouse cursor
    SetMouseCursor(MOUSE_CURSOR_NORMAL);

    return true;
}

void Drawing::SetMouseCursor(AppMouseCursors type)
{
    if (type == MAX_MOUSE_CURSOR || !m_mouseCursors[type])
        return;

    SDL_SetCursor(m_mouseCursors[type]->GetSDLCursor());
    m_currentMouseCursor = type;
}

AppMouseCursors Drawing::GetCurrentMouseCursor()
{
    return m_currentMouseCursor;
}

void Drawing::SetCanvasRedrawFlag()
{
    m_canvasRedraw = true;
}

void Drawing::Update()
{
    // increase FPS counter
    m_fpsCounter++;

    // update FPS once every second
    if (getMSTimeDiff(m_lastUpdate, getMSTime()) > 1000)
    {
        std::string title("BubbleWorld; FPS: ");
        title += std::to_string(m_fpsCounter);
        SDL_SetWindowTitle(m_window, title.c_str());
        m_fpsCounter = 0;
        m_lastUpdate = getMSTime();
    }

    // do we need to redraw canvas? if not, return
    if (!m_canvasRedraw)
        return;

    // should we draw world?
    if (m_drawWorld)
        DrawWorld();
    else // the world covers whole canvas, so we clear double-buffering buffer only when not drawing world
        SDL_RenderClear(m_renderer);

    // lock scope for UI drawing
    {
        std::unique_lock<std::mutex> lck(m_widgetListMtx);
        for (std::list<UIWidget*>::iterator itr = m_widgetList.begin(); itr != m_widgetList.end(); ++itr)
            (*itr)->OnDraw();
    }

    // put contents from buffer to screen
    SDL_RenderPresent(m_renderer);

    m_canvasRedraw = false;
}

void Drawing::DrawWorld()
{
    uint32_t cellX, cellY, beginX, beginY, endX, endY;
    int32_t itX, itY;
    WorldObject* obj;
    Player* plr = sGameplay->GetPlayer();
    Map* map = sGameplay->GetMap();

    // retrieve player cell (to draw only sorroundings)
    cellX = map->GetChunkIndexX((uint32_t)(plr->GetPositionX()));
    cellY = map->GetChunkIndexY((uint32_t)(plr->GetPositionY()));

    // retrieve sorrounding cell limits
    map->GetCellSorroundingLimits(cellX, cellY, beginX, beginY, endX, endY);

    // determine absolute field limits
    beginX = map->GetChunkStartX(beginX);
    beginY = map->GetChunkStartY(beginY);
    endX = map->GetChunkStartX(endX - 1) + MAP_CHUNK_SIZE_X - 1;
    endY = map->GetChunkStartY(endY - 1) + MAP_CHUNK_SIZE_Y - 1;

    MapField const* fld;
    SDL_Texture* texture;
    ImageResource* imgres;
    SDL_Rect target;
    uint32_t textureId;

    // cache several positions
    int32_t px = (int32_t)((float)MAP_FIELD_PX_SIZE_X * plr->GetPositionX());
    int32_t py = (int32_t)((float)MAP_FIELD_PX_SIZE_Y * plr->GetPositionY());
    int32_t whalf = m_windowWidth / 2;
    int32_t hhalf = m_windowHeight / 2;

    // base for drawing - "move view there"
    int32_t baseX = (whalf > px) ? 0 : (-px + whalf);
    int32_t baseY = (hhalf > py) ? 0 : (-py + hhalf);

    // for fields drawing, we use constant width/height values
    target.w = MAP_FIELD_PX_SIZE_X;
    target.h = MAP_FIELD_PX_SIZE_Y;

    // draw map
    for (itX = (int32_t)beginX; itX < (int32_t)endX; itX++)
    {
        for (itY = (int32_t)beginY; itY < (int32_t)endY; itY++)
        {
            // retrieve field; unsafe option to be faster (no checks, we have to be sure there)
            fld = map->GetField_unsafe(itX, itY);

            // determine starting position
            target.x = baseX + itX * MAP_FIELD_PX_SIZE_X;
            target.y = baseY + itY * MAP_FIELD_PX_SIZE_Y;

            // when the field is not within view, do not draw
            if (target.x + target.w < 0 || target.y + target.h < 0 || target.x > m_windowWidth || target.y > m_windowHeight)
                continue;

            textureId = fld->texture;

            // get texture and draw it
            texture = sResourceManager->GetImage(textureId);
            if (texture)
            {
                imgres = sResourceManager->GetImageRecord(textureId);
                // the texture should have at least one "frame" to be drawn; sprite rect is clipped to width/height
                if (imgres && imgres->animSpriteRects.size() != 0)
                    DrawTexture(texture, &imgres->animSpriteRects[0], &target);
            }
        }
    }

    SDL_Texture* tmptxt;

    // draw objects on map
    ObjectVector const& objvect = map->GetObjectVisibilityVector();
    for (uint32_t i = 0; i < objvect.size(); i++)
    {
        obj = objvect[i];

        // get image ID
        textureId = obj->GetUInt32Value(OBJECT_FIELD_IMAGEID);

        if (textureId != 0)
        {
            texture = sResourceManager->GetImage(textureId);
            if (texture)
            {
                imgres = sResourceManager->GetImageRecord(textureId);
                if (imgres && imgres->animSpriteRects.size() != 0)
                {
                    // determine position, move by view, move to be placed into "texture base center"
                    target.x = baseX + (uint32_t)((float)MAP_FIELD_PX_SIZE_X * obj->GetPositionX()) - imgres->metadata->baseCenterX;
                    target.y = baseY + (uint32_t)((float)MAP_FIELD_PX_SIZE_Y * obj->GetPositionY()) - imgres->metadata->baseCenterY;
                    target.w = imgres->metadata->sizeX;
                    target.h = imgres->metadata->sizeY;
                    // draw
                    DrawTexture(texture, &imgres->animSpriteRects[obj->GetAnimFrame()], &target);

                    // if it's unit
                    if (obj->GetType() == OTYPE_CREATURE || obj->GetType() == OTYPE_PLAYER)
                    {
                        // draw name if available
                        if (tmptxt = obj->GetNameTexture())
                        {
                            SDL_QueryTexture(tmptxt, nullptr, nullptr, &target.w, &target.h);
                            target.x = baseX + (uint32_t)((float)MAP_FIELD_PX_SIZE_X * obj->GetPositionX()) - target.w / 2;
                            target.y = target.y - target.h;
                            DrawTexture(tmptxt, &target);
                        }

                        // draw chat if there's any chat message to be drawn
                        if (tmptxt = obj->ToUnit()->GetDisplayChat())
                        {
                            SDL_QueryTexture(tmptxt, nullptr, nullptr, &target.w, &target.h);
                            target.x = baseX + (uint32_t)((float)MAP_FIELD_PX_SIZE_X * obj->GetPositionX()) - target.w / 2;
                            target.y = target.y - target.h;

                            // draw rounded box for text
                            roundedBoxRGBA(m_renderer, target.x - 12, target.y - 7, target.x + target.w + 12, target.y + target.h + 7, 5, 255, 255, 255, 255);
                            // draw rounded box border
                            roundedRectangleRGBA(m_renderer, target.x - 12, target.y - 7, target.x + target.w + 12, target.y + target.h + 7, 5, 127, 127, 127, 255);
                            // draw triangle on the bottom
                            filledTrigonRGBA(m_renderer,
                                target.x + target.w / 2 - 10, target.y + target.h + 7,
                                target.x + target.w / 2 + 10, target.y + target.h + 7,
                                target.x + target.w / 2, target.y + target.h + 15, 255, 255, 255, 255);
                            // draw "border" line of the triangle
                            lineRGBA(m_renderer, target.x + target.w / 2 - 10, target.y + target.h + 7,
                                target.x + target.w / 2, target.y + target.h + 15, 127, 127, 127, 255);
                            // draw second "border" line of the triangle
                            lineRGBA(m_renderer, target.x + target.w / 2 + 10, target.y + target.h + 7,
                                target.x + target.w / 2, target.y + target.h + 15, 127, 127, 127, 255);

                            // finally draw text
                            DrawTexture(tmptxt, &target);
                        }
                    }
                }
            }
        }
    }

    // draw chat frame
    target.x = 10;
    target.y = m_windowHeight - 20 - CHAT_MSG_FRAME_HEIGHT - 10;
    target.w = CHAT_MSG_FRAME_WIDTH + 10 + 10;
    target.h = CHAT_MSG_FRAME_HEIGHT + 10 + 10;
    boxRGBA(m_renderer, 10, m_windowHeight - 20 - CHAT_MSG_FRAME_HEIGHT - 10, CHAT_MSG_FRAME_WIDTH + 10 + 10 + 10, m_windowHeight - 10,
        255, 255, 255, 127);

    // draw chat message history
    target.x = 20;
    target.y = m_windowHeight - 20;
    std::list<ChatMessageRecord> const& msglist = sGameplay->GetChatMessages();
    for (std::list<ChatMessageRecord>::const_reverse_iterator itr = msglist.rbegin(); itr != msglist.rend(); ++itr)
    {
        ChatMessageRecord const& cmsg = *itr;

        SDL_QueryTexture(cmsg.texture, nullptr, nullptr, &target.w, &target.h);
        target.y -= target.h;
        if (m_windowHeight - 20 - target.y > CHAT_MSG_FRAME_HEIGHT)
            break;
        DrawTexture(cmsg.texture, &target);
    }
}

void Drawing::AddUIWidget(UIWidget* widget)
{
    // remove any other splashmessage widgets, when adding splash message
    if (widget->GetWidgetType() == UIWIDGET_SPLASHMESSAGE)
        RemoveUIWidgetsOfType(UIWIDGET_SPLASHMESSAGE);

    // lock scope
    {
        std::unique_lock<std::mutex> lck(m_widgetListMtx);

        m_widgetList.push_back(widget);
    }

    // just for being sure everything works as intended
    widget->UpdateCanvas();

    SetCanvasRedrawFlag();
}

void Drawing::RemoveUIWidget(UIWidget* widget)
{
    // erase from UI widget list
    for (std::list<UIWidget*>::iterator itr = m_widgetList.begin(); itr != m_widgetList.end();)
    {
        if (*itr == widget)
            itr = m_widgetList.erase(itr);
        else
            ++itr;
    }

    // if the element had focus or was marked as hover, remove it

    if (widget == m_hoverElement)
        m_hoverElement = nullptr;
    if (widget == m_focusElement)
        m_focusElement = nullptr;

    SetCanvasRedrawFlag();
}

void Drawing::RemoveUIWidgetsOfType(UIWidgetType type)
{
    std::list<UIWidget*> toRemove;

    // lock scope
    {
        std::unique_lock<std::mutex> lck(m_widgetListMtx);
        // look for UI widgets to be removed
        for (std::list<UIWidget*>::iterator itr = m_widgetList.begin(); itr != m_widgetList.end(); ++itr)
        {
            if ((*itr)->GetWidgetType() == type)
                toRemove.push_back(*itr);
        }
    }

    // remove them
    for (std::list<UIWidget*>::iterator itr = toRemove.begin(); itr != toRemove.end(); ++itr)
    {
        RemoveUIWidget(*itr);
        delete *itr;
    }
}

void Drawing::DestroyUI()
{
    std::unique_lock<std::mutex> lck(m_widgetListMtx);

    // after this, no hover or focus element will be present
    m_hoverElement = nullptr;
    m_focusElement = nullptr;

    // delete all widgets from list
    for (std::list<UIWidget*>::iterator itr = m_widgetList.begin(); itr != m_widgetList.end(); ++itr)
        delete *itr;

    // clear widget list
    m_widgetList.clear();

    SetCanvasRedrawFlag();
}

UIWidget* Drawing::FindUIWidgetById(uint32_t id)
{
    for (std::list<UIWidget*>::iterator itr = m_widgetList.begin(); itr != m_widgetList.end(); ++itr)
    {
        if ((*itr)->GetID() == id)
            return *itr;
    }

    return nullptr;
}

void Drawing::OnMouseMove(int32_t x, int32_t y)
{
    std::unique_lock<std::mutex> lck(m_widgetListMtx);

    // leaving current hover element?
    if (m_hoverElement && !m_hoverElement->IsCoordsInside(x, y))
    {
        m_hoverElement->OnMouseLeave();
        m_hoverElement = nullptr;
    }

    // use reverse iterator - UIElement at the bottom of list is the top element on screen
    for (std::list<UIWidget*>::reverse_iterator itr = m_widgetList.rbegin(); itr != m_widgetList.rend(); ++itr)
    {
        // if the element is interactive and the mouse is inside...
        if ((*itr)->IsInteractive() && (*itr)->IsCoordsInside(x, y))
        {
            // if the element is already marked as hover element, do nothing
            if (m_hoverElement == *itr)
                return;

            // if there was any hover element before, call leave event method
            if (m_hoverElement)
                m_hoverElement->OnMouseLeave();

            // step into new hover element
            m_hoverElement = (*itr);
            (*itr)->OnMouseEnter();
            break;
        }
    }
}

void Drawing::OnMouseClick(bool left, bool press)
{
    std::unique_lock<std::mutex> lck(m_widgetListMtx);

    // we call it only on hover element
    if (!m_hoverElement)
        return;

    m_hoverElement->OnMouseClick(left, press);

    // note: the element could remove itself in OnMouseClick call
    // therefore m_hoverElement would be nullptr

    if (m_hoverElement && m_focusElement != m_hoverElement)
        SetFocusElement(m_hoverElement);
}

void Drawing::SetFocusElement(UIWidget* widget)
{
    // leave current focus element if any
    if (m_focusElement)
        m_focusElement->OnBlur();

    m_focusElement = widget;

    // if we are not nullifying focus element, call focus event method
    if (m_focusElement)
        m_focusElement->OnFocus();
}

bool Drawing::OnKeyPress(int key, bool press)
{
    std::unique_lock<std::mutex> lck(m_widgetListMtx);

    // we propagate keypress event only to focused elements
    if (!m_focusElement)
        return false;

    return m_focusElement->OnKeyPress(key, press);
}

SDL_Surface* Drawing::RenderFont(AppFonts fontId, const char* message, SDL_Color color)
{
    return TTF_RenderText_Solid(m_fonts[fontId], message, color);
}

SDL_Surface* Drawing::RenderFontWrapped(AppFonts fontId, const char* message, uint32_t width, SDL_Color color)
{
    return TTF_RenderText_Blended_Wrapped(m_fonts[fontId], message, color, width);
}

uint16_t Drawing::GetFontHeight(AppFonts fontId, bool useAscent)
{
    // use font metrics if requested
    if (useAscent)
        return TTF_FontAscent(m_fonts[fontId]);
    // otherwise retrieve size from options
    return appFontParams[fontId].size;
}

void Drawing::GetTextureSize(SDL_Texture* texture, int* w, int* h)
{
    SDL_QueryTexture(texture, nullptr, nullptr, w, h);
}

void Drawing::SetDrawWorld(bool state)
{
    m_drawWorld = state;
}

void Drawing::DrawTexture(SDL_Texture* texture, SDL_Rect* source, SDL_Rect* target)
{
    SDL_RenderCopy(m_renderer, texture, source, target);
}

void Drawing::DrawTexture(SDL_Texture* texture, SDL_Rect* target)
{
    DrawTexture(texture, nullptr, target);
}

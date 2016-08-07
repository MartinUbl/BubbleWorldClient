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
#include "MouseCursor.h"
#include "Log.h"

MouseCursor::MouseCursor()
{
    m_cursor = nullptr;
}

MouseCursor::~MouseCursor()
{
    //
}

MouseCursor* MouseCursor::LoadFromFile(const char* cursorImageFilename)
{
    // load cursor image
    std::string path = cursorImageFilename;
    SDL_Surface* imgsurface = IMG_Load(path.c_str());
    if (!imgsurface)
    {
        sLog->Error("Could not load cursor image file %s", path.c_str());
        return nullptr;
    }

    int32_t coordX, coordY;

    // load cursor metadata
    std::string metapath = path + ".cursormeta";
    FILE* f = fopen(metapath.c_str(), "r");
    if (!f || fscanf(f, "%i,%i", &coordX, &coordY) != 2)
    {
        sLog->Error("Failed to load cursor metadata from %s, using default values", metapath.c_str());

        coordX = 0;
        coordY = 0;
    }

    // secure coordinate bounds
    if (coordX >= imgsurface->w || coordX < 0 || coordY >= imgsurface->h || coordY < 0)
    {
        sLog->Error("Invalid active coordinate values %i %i, using default values", coordX, coordY);

        coordX = 0;
        coordY = 0;
    }

    // create cursor instance
    MouseCursor* cur = new MouseCursor();

    cur->m_activeX = coordX;
    cur->m_activeY = coordY;
    cur->m_cursor = SDL_CreateColorCursor(imgsurface, coordX, coordY);

    return cur;
}

SDL_Cursor* MouseCursor::GetSDLCursor()
{
    return m_cursor;
}

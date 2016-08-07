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

#ifndef BW_MOUSECURSOR_H
#define BW_MOUSECURSOR_H

/*
 * Class holding mouse cursor data
 */
class MouseCursor
{
    public:
        virtual ~MouseCursor();

        // factory method for loading cursor from file
        static MouseCursor* LoadFromFile(const char* cursorImageFilename);
        // retrieves built SDL cursor from loaded data
        SDL_Cursor* GetSDLCursor();

    protected:
        // protected constructor; use LoadFromFile factory method to instantiate
        MouseCursor();

    private:
        // prepared SDL cursor
        SDL_Cursor* m_cursor;
        // active point X coordinate
        int32_t m_activeX;
        // active point Y coordinate
        int32_t m_activeY;
};

#endif

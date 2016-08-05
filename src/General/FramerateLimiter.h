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

#ifndef BW_FRAMERATE_LIMITER_H
#define BW_FRAMERATE_LIMITER_H

#include "Singleton.h"

/*
 * Singleton class maintaining FPS limit
 */
class FramerateLimiter
{
    friend class Singleton<FramerateLimiter>;
    public:
        // initialize FPS limiter
        void Initialize(uint32_t maxFramerate);
        // limit framerate if needed
        void Limit();

    protected:
        // protected singleton constructor
        FramerateLimiter();

    private:
        // limit of ticks
        float frameTickLimit;
        // ticks within last second
        uint32_t lastSecTicks;
        // current frame count
        uint32_t frameCounter;
};

#define sFramerateLimiter Singleton<FramerateLimiter>::getInstance()

#endif

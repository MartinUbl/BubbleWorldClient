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
#include "FramerateLimiter.h"

FramerateLimiter::FramerateLimiter()
{
    //
}

void FramerateLimiter::Initialize(uint32_t maxFramerate)
{
    frameTickLimit = (1000.0f / (float)maxFramerate) + 0.00001f;
    frameCounter = 0;
    lastSecTicks = SDL_GetTicks();
}

void FramerateLimiter::Limit()
{
    frameCounter++;

    uint32_t targetTicks = lastSecTicks + uint32_t(frameCounter * frameTickLimit);
    uint32_t currentTicks = SDL_GetTicks();

    if (currentTicks < targetTicks)
    {
        SDL_Delay(targetTicks - currentTicks);
        currentTicks = SDL_GetTicks();
    }

    if (currentTicks - lastSecTicks >= 1000)
    {
        frameCounter = 0;
        lastSecTicks = SDL_GetTicks();
    }
}

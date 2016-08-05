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

#ifndef BW_GENERAL_H
#define BW_GENERAL_H

// common headers
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <set>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdint.h>

// platform-specific headers
#ifdef _WIN32
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

// library headers
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL2_gfxPrimitives.h"

// compatibility and global headers
#include "GlobalActions.h"
#include "SharedEnums.h"
#include "Compatibility.h"

// global defines
#ifdef _DEBUG
#define DATA_DIR "bin/data/"
#define FONTS_DATA_DIR "bin/data/fonts/"
#else
#define DATA_DIR "data/"
#define FONTS_DATA_DIR "data/fonts/"
#endif

// version
#define APP_VERSION 1
#define APP_VERSION_STR "v0.1"

#endif

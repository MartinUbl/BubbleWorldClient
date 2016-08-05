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

#ifndef BW_GLOBALACTIONS_H
#define BW_GLOBALACTIONS_H

// enumerator of global actions broadcast through application
enum GlobalActionIDs
{
    // MenuStage
    GA_CONNECTION_START             = 1,
    GA_CONNECTION_CONNECTED,
    GA_CONNECTION_DISCONNECTED,
    GA_CONNECTION_FETCHING,
    GA_CONNECTION_SUCCESS,
    GA_CONNECTION_UNABLE_TO_CONNECT,
    GA_CONNECTION_INVALID_USER,
    GA_CONNECTION_INVALID_PASSWORD,
    GA_CONNECTION_INCOMPATIBLE_VERSION,
    GA_CONNECTION_BANNED,

    // LobbyStage
    GA_CHARACTER_LIST_ACQUIRED,
};

#endif

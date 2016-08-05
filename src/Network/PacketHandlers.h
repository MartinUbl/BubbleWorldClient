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

#ifndef BW_PACKETHANDLERS_H
#define BW_PACKETHANDLERS_H

#include "SmartPacket.h"

// packet handler function arguments
#define PACKET_HANDLER_ARGS SmartPacket &packet
// packet handler definition
#define PACKET_HANDLER(x) void x(PACKET_HANDLER_ARGS)

// some prepared state restriction masks
enum StateRestrictionMask
{
    STATE_RESTRICTION_NEVER         = 0,
    STATE_RESTRICTION_ANY           = (uint32_t)(-1),
    STATE_RESTRICTION_AUTH          = 1 << CONNECTION_STATE_AUTH,
    STATE_RESTRICTION_LOBBY         = 1 << CONNECTION_STATE_LOBBY,
    STATE_RESTRICTION_GAME          = 1 << CONNECTION_STATE_INGAME,
    STATE_RESTRICTION_VERIFIED      = 1 << CONNECTION_STATE_LOBBY | 1 << CONNECTION_STATE_INGAME,
};

/*
 * Structure of packet handler record
 */
struct PacketHandlerStructure
{
    // handler function
    void (*handler)(PACKET_HANDLER_ARGS);

    // state restriction
    StateRestrictionMask stateRestriction;
};

// we wrap all packet handlers into namespace
namespace PacketHandlers
{
    PACKET_HANDLER(Handle_NULL);
    PACKET_HANDLER(Handle_ServerSide);

    PACKET_HANDLER(HandleLoginResponse);
    PACKET_HANDLER(HandleCharacterList);
    PACKET_HANDLER(HandleResourceSendStart);
    PACKET_HANDLER(HandleResourceSendFinished);
    PACKET_HANDLER(HandleResourceData);
    PACKET_HANDLER(HandleResourceChecksumVerify);
    PACKET_HANDLER(HandleEnterWorldResult);
    PACKET_HANDLER(HandleCreateObject);
    PACKET_HANDLER(HandleUpdateObject);
    PACKET_HANDLER(HandleDestroyObject);
    PACKET_HANDLER(HandleMapMetadata);
    PACKET_HANDLER(HandleMapChunk);
    PACKET_HANDLER(HandleMapMetaChecksumVerify);
    PACKET_HANDLER(HandleMapChunkChecksumVerify);
    PACKET_HANDLER(HandleImageMetadata);
    PACKET_HANDLER(HandleImageMetaChecksumVerify);
    PACKET_HANDLER(HandleNameQueryResponse);
    PACKET_HANDLER(HandleMoveStartDir);
    PACKET_HANDLER(HandleMoveStopDir);
    PACKET_HANDLER(HandleMoveHeartbeat);
    PACKET_HANDLER(HandleChatMessage);
};

// table of packet handlers; the opcode is also an index here
static PacketHandlerStructure PacketHandlerTable[] = {
    { &PacketHandlers::Handle_NULL,             STATE_RESTRICTION_NEVER },      // OPCODE_NONE
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_LOGIN_REQUEST
    { &PacketHandlers::HandleLoginResponse,     STATE_RESTRICTION_AUTH },       // SP_LOGIN_RESPONSE
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_CHARACTER_LIST_REQUEST
    { &PacketHandlers::HandleCharacterList,     STATE_RESTRICTION_LOBBY },      // SP_CHARACTER_LIST
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_REQUEST_RESOURCE
    { &PacketHandlers::HandleResourceSendStart, STATE_RESTRICTION_ANY   },      // SP_RESOURCE_SEND_START
    { &PacketHandlers::HandleResourceSendFinished, STATE_RESTRICTION_ANY },     // SP_RESOURCE_SEND_FINISHED
    { &PacketHandlers::HandleResourceData,      STATE_RESTRICTION_ANY   },      // SP_RESOURCE_DATA
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_RESOURCE_VERIFY_CHECKSUM
    { &PacketHandlers::HandleResourceChecksumVerify, STATE_RESTRICTION_ANY },   // SP_RESOURCE_VERIFY_CHECKSUM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_ENTER_WORLD
    { &PacketHandlers::HandleEnterWorldResult,  STATE_RESTRICTION_LOBBY },      // SP_ENTER_WORLD_RESULT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_WORLD_ENTER_COMPLETE
    { &PacketHandlers::HandleCreateObject,      STATE_RESTRICTION_VERIFIED },   // SP_CREATE_OBJECT
    { &PacketHandlers::HandleUpdateObject,      STATE_RESTRICTION_VERIFIED },   // SP_UPDATE_OBJECT
    { &PacketHandlers::HandleDestroyObject,     STATE_RESTRICTION_VERIFIED },   // SP_DESTROY_OBJECT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_GET_MAP_METADATA
    { &PacketHandlers::HandleMapMetadata,       STATE_RESTRICTION_VERIFIED },   // SP_MAP_METADATA
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_GET_MAP_CHUNK
    { &PacketHandlers::HandleMapChunk,          STATE_RESTRICTION_VERIFIED },   // SP_MAP_CHUNK
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_MAP_METADATA_VERIFY_CHECKSUM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_MAP_CHUNK_VERIFY_CHECKSUM
    { &PacketHandlers::HandleMapMetaChecksumVerify,     STATE_RESTRICTION_VERIFIED },   // SP_MAP_METADATA_VERIFY_CHECKSUM
    { &PacketHandlers::HandleMapChunkChecksumVerify,    STATE_RESTRICTION_VERIFIED },   // SP_MAP_CHUNK_VERIFY_CHECKSUM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_GET_IMAGE_METADATA
    { &PacketHandlers::HandleImageMetadata,     STATE_RESTRICTION_VERIFIED },   // SP_IMAGE_METADATA
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_VERIFY_IMAGE_METADATA_CHECKSUM
    { &PacketHandlers::HandleImageMetaChecksumVerify,   STATE_RESTRICTION_VERIFIED },   // SP_VERIFY_IMAGE_METADATA_CHECKSUM
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_NAME_QUERY
    { &PacketHandlers::HandleNameQueryResponse, STATE_RESTRICTION_VERIFIED },   // SP_NAME_QUERY_RESPONSE
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_MOVE_START_DIRECTION
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_MOVE_STOP_DIRECTION
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_MOVE_HEARTBEAT
    { &PacketHandlers::HandleMoveStartDir,      STATE_RESTRICTION_GAME  },      // SP_MOVE_START_DIRECTION
    { &PacketHandlers::HandleMoveStopDir,       STATE_RESTRICTION_GAME  },      // SP_MOVE_STOP_DIRECTION
    { &PacketHandlers::HandleMoveHeartbeat,     STATE_RESTRICTION_GAME  },      // SP_MOVE_HEARTBEAT
    { &PacketHandlers::Handle_ServerSide,       STATE_RESTRICTION_NEVER },      // CP_CHAT_MESSAGE
    { &PacketHandlers::HandleChatMessage,       STATE_RESTRICTION_GAME },       // SP_CHAT_MESSAGE
};

#endif

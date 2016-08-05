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
#include "Application.h"
#include "NetworkManager.h"
#include "PacketHandlers.h"
#include "Log.h"

#include <sstream>

NetworkManager::NetworkManager()
{
    m_connected = false;
    m_disconnectFlag = false;
    m_running = false;
}

NetworkManager::~NetworkManager()
{
    //
}

bool NetworkManager::Init()
{
#ifdef _WIN32
    // on Windows, we need to initialize WinSock service
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(version, &data) != 0)
    {
        sLog->Error("Unable to start winsock service");
        return false;
    }
#endif

    // reset flags
    m_connected = false;
    m_running = true;
    m_disconnectFlag = false;

    // spawn networking thread
    m_networkThread = new std::thread(&NetworkManager::Update, this);
    if (!m_networkThread)
        return false;

    return true;
}

void NetworkManager::Update()
{
    fd_set rdset;
    // we will read just packet header, and then packet data if any
    struct
    {
        uint16_t opcode;
        uint16_t size;
    } recvHeader;
    // buffer
    uint8_t recvDataBuffer[RECV_DATA_BUFFER_SIZE];

    int res;

    // set timeout to 1s
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 1;

    // main networking loop
    while (m_running)
    {
        // while not connected, perform connection routine
        while (!m_connected)
        {
            // at first, wait on condition - thread is signaled when user clicks on "login"
            std::unique_lock<std::mutex> lck(m_connectionMtx);
            m_connectionCond.wait(lck);

            // broadcast about connection
            sApplication->SignalGlobalEvent(GA_CONNECTION_START);

            // init socket
            m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            m_sockAddr.sin_family = AF_INET;
            m_sockAddr.sin_port = htons(m_port);

            // resolve remote address
            if (INET_PTON(AF_INET, m_host.c_str(), &m_sockAddr.sin_addr.s_addr) != 1)
            {
                sLog->Error("Unable to resolve remote address");
                sApplication->SignalGlobalEvent(GA_CONNECTION_UNABLE_TO_CONNECT);
                m_connected = false;
                continue;
            }

            // connect
            if (connect(m_socket, (sockaddr*)&m_sockAddr, sizeof(sockaddr_in)) < 0)
            {
                sLog->Error("Unable to connect to server");
                sApplication->SignalGlobalEvent(GA_CONNECTION_UNABLE_TO_CONNECT);
                m_connected = false;
                continue;
            }

            // yay! we are connected, set connection state and broadcast event
            m_connected = true;
            SetConnectionState(CONNECTION_STATE_AUTH);
            sApplication->SignalGlobalEvent(GA_CONNECTION_CONNECTED);
        }

        // connected loop - select() on socket, process data if any, loop until ending is requested
        while (m_running && !m_disconnectFlag)
        {
            FD_ZERO(&rdset);
            FD_SET(m_socket, &rdset);
            tv.tv_sec = 1;
            tv.tv_usec = 1;

            res = select((int)m_socket + 1, &rdset, nullptr, nullptr, &tv);
            // error
            if (res < 0)
            {
                sLog->Error("select(): error %u", LASTERROR());
            }
            // something's on input
            else if (res > 0)
            {
                if (FD_ISSET(m_socket, &rdset))
                {
                    // read initial bytes (header)
                    res = recv(m_socket, (char*)&recvHeader, SmartPacket::HeaderSize, 0);
                    // sanitize length
                    if (res < SmartPacket::HeaderSize)
                    {
                        sLog->Error("recv(): error, received malformed packet");
                        // unrecoverable error, disconnect
                        m_disconnectFlag = true;
                        continue;
                    }
                    else
                    {
                        // retrieve opcode and size
                        recvHeader.opcode = ntohs(recvHeader.opcode);
                        recvHeader.size = ntohs(recvHeader.size);

                        if (recvHeader.size > 0 && recvHeader.size < RECV_DATA_BUFFER_SIZE)
                        {
                            int recbytes = 0;

                            // while there's something to read..
                            while (recbytes != recvHeader.size)
                            {
                                // retrieve next bunch
                                res = recv(m_socket, (char*)(recvDataBuffer + recbytes), recvHeader.size - recbytes, 0);
                                if (res <= 0)
                                {
                                    if (LASTERROR() == SOCKETWOULDBLOCK)
                                        continue;
                                    sLog->Error("recv(): error, received packet with different size than expected");
                                    recbytes = -1;
                                    break;
                                }
                                recbytes += res;
                            }

                            if (recbytes == -1)
                                continue;
                        }
                        else if (recvHeader.size >= RECV_DATA_BUFFER_SIZE)
                        {
                            sLog->Error("recv(): error, received bigger packet than expected, exiting to avoid overflow");
                            // unrecoverable error, disconnect
                            m_disconnectFlag = true;
                            continue;
                        }

                        // queue it

                        std::unique_lock<std::mutex> lck(m_packetQueueMtx);

                        // build packet
                        PendingPacket* pp = new PendingPacket();
                        pp->pkt = new SmartPacket(recvHeader.opcode, recvHeader.size);
                        pp->pkt->SetData(recvDataBuffer, recvHeader.size);
                        pp->timeArrived = getMSTime();

                        // put it into queue
                        m_packetQueue.push(pp);
                    }
                }
            }
        }

        // if still supposed to run, and disconnection was requested, clear state, broadcast event and start again
        if (m_running && m_disconnectFlag)
        {
            std::unique_lock<std::mutex> lck(m_connectionMtx);

            SetConnectionState(CONNECTION_STATE_NONE);
            m_connected = false;
            sApplication->SignalGlobalEvent(GA_CONNECTION_DISCONNECTED);
            continue;
        }
    }
}

void NetworkManager::Connect(const char* host, uint16_t port)
{
    std::unique_lock<std::mutex> lck(m_connectionMtx);

    m_disconnectFlag = false;

    m_host = host;
    m_port = port;

    // notify networking thread to perform connection routine
    m_connectionCond.notify_all();
}

void NetworkManager::Disconnect()
{
    m_disconnectFlag = true;

    // to immediatelly cut select()
#ifdef _WIN32
    closesocket(m_socket);
#else
    close(m_socket);
#endif
}

void NetworkManager::SendPacket(SmartPacket& pkt)
{
    uint16_t op, sz;
    uint8_t* tosend = new uint8_t[SmartPacket::HeaderSize + pkt.GetSize()];

    op = htons(pkt.GetOpcode());
    sz = htons(pkt.GetSize());

    // write opcode
    memcpy(tosend, &op, 2);
    // write contents size
    memcpy(tosend + 2, &sz, 2);
    // write contents
    memcpy(tosend + 4, pkt.GetData(), pkt.GetSize());

    // send response
    send(m_socket, (const char*)tosend, pkt.GetSize() + SmartPacket::HeaderSize, MSG_NOSIGNAL);
}

void NetworkManager::ProcessPending()
{
    PendingPacket* pp;
    std::unique_lock<std::mutex> lck(m_packetQueueMtx);

    // work while there's something to process
    while (!m_packetQueue.empty())
    {
        // pop packet
        pp = m_packetQueue.front();
        m_packetQueue.pop();

        // handle
        HandlePacket(*pp->pkt);

        // cleanup
        delete pp;
    }
}

void NetworkManager::SetConnectionState(ConnectionState state)
{
    m_connectionState = state;
}

void NetworkManager::HandlePacket(SmartPacket &packet)
{
    // do not handle opcodes higher than maximum
    if (packet.GetOpcode() >= MAX_OPCODES)
    {
        sLog->Error("Server sent invalid packet (unknown opcode %u), not handling", packet.GetOpcode());
        return;
    }

    // packet handlers might throw exception about trying to reach out of packet data range
    try
    {
        // verify the state of client connection
        if ((PacketHandlerTable[packet.GetOpcode()].stateRestriction & (1 << (int)m_connectionState)) == 0)
        {
            sLog->Error("Server sent invalid packet (opcode %u) for state %u, not handling", packet.GetOpcode(), m_connectionState);
            return;
        }

        // look handler up in handler table and call it
        PacketHandlerTable[packet.GetOpcode()].handler(packet);
    }
    catch (PacketReadException &ex)
    {
        sLog->Error("Read error during executing handler for opcode %u - attempt to read %u bytes at offset %u (real size %u bytes)", packet.GetOpcode(), ex.GetAttemptSize(), ex.GetPosition(), packet.GetSize());
    }
}

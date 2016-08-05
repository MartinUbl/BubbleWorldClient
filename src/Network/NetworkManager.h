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

#ifndef BW_NETWORKMANAGER_H
#define BW_NETWORKMANAGER_H

#include "Singleton.h"
#include "SmartPacket.h"

// platform-dependent defines and includes
#ifdef _WIN32
#define SOCK SOCKET
#define ADDRLEN int

#define SOCKETWOULDBLOCK WSAEWOULDBLOCK
#define SOCKETCONNRESET  WSAECONNRESET
#define SOCKETCONNABORT  WSAECONNABORTED
#define SOCKETINPROGRESS WSAEINPROGRESS
#define LASTERROR() WSAGetLastError()
#define INET_PTON(fam,addrptr,buff) InetPtonA(fam,addrptr,buff)
#define INET_NTOP(fam,addrptr,buff,socksize) InetNtopA(fam,addrptr,buff,socksize)
#define CLOSESOCKET closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <netdb.h>
#include <fcntl.h>

#define SOCK int
#define ADDRLEN socklen_t

#define INVALID_SOCKET -1

#define SOCKETWOULDBLOCK EAGAIN
#define SOCKETCONNABORT ECONNABORTED
#define SOCKETCONNRESET ECONNRESET
#define SOCKETINPROGRESS EINPROGRESS
#define LASTERROR() errno
#define INET_PTON(fam,addrptr,buff) inet_pton(fam,addrptr,buff)
#define INET_NTOP(fam,addrptr,buff,socksize) inet_ntop(fam,addrptr,buff,socksize)
#define CLOSESOCKET close
#endif

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

// 64kB receive buffer
#define RECV_DATA_BUFFER_SIZE   64*1024

/*
 * Pending packet structure
 */
struct PendingPacket
{
    // received packet
    SmartPacket *pkt;
    // time of its arrival
    uint32_t timeArrived;
};

/*
 * Singleton class maintaining network communication
 */
class NetworkManager
{
    friend class Singleton<NetworkManager>;
    public:
        ~NetworkManager();

        // initialize networking
        bool Init();
        // update method is called from separate thread
        void Update();
        // ProcessPending is method called from main thread - processes received packets
        void ProcessPending();

        // connect to server
        void Connect(const char* host, uint16_t port);
        // disconnect from server
        void Disconnect();
        // send packet to server
        void SendPacket(SmartPacket &pkt);
        // set connection state (security and sanity reasons)
        void SetConnectionState(ConnectionState state);

    protected:
        // protected singleton constructor
        NetworkManager();
        // handles incoming packet and puts it into queue
        void HandlePacket(SmartPacket &pkt);

    private:
        // network thread handle pointer
        std::thread* m_networkThread;
        // mutex for packet queue
        std::mutex m_packetQueueMtx;
        // mutex for connection monitor operations
        std::mutex m_connectionMtx;
        // packet queue to be processed
        std::queue<PendingPacket*> m_packetQueue;
        // connection monitor condition variable
        std::condition_variable m_connectionCond;

        // host to connect to
        std::string m_host;
        // port to connect to
        uint16_t m_port;
        // is connected to host?
        bool m_connected;
        // is still supposed to run?
        bool m_running;
        // did we request disconnection?
        bool m_disconnectFlag;
        // current connection state
        ConnectionState m_connectionState;

        // client socket
        SOCK m_socket;
        // socket addr struct
        sockaddr_in m_sockAddr;
};

#define sNetwork Singleton<NetworkManager>::getInstance()

#endif

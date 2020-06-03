/*
 UIPServer.h - Arduino implementation of a UIP wrapper class.
 Copyright (c) 2013 Norbert Truchsess <norbert.truchsess@t-online.de>
 All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
#ifndef TCPSERVER_h
#define TCPSERVER_h

#include "TcpClient.h"

class UipEthernet;

class TcpServer
{
public:
    TcpServer();
    void        open(UipEthernet* ethernet);
    void        bind(uint8_t port);
    void        bind(const char* ip, uint8_t port);
    void        listen(uint8_t conns);
    TcpClient*  accept();
    size_t      send(uint8_t);
    size_t      send(const uint8_t* buf, size_t size);
private:
    uint16_t    _port;
    uint8_t     _conns;
};
#endif

/*
 TcpServer.cpp - Arduino implementation of a UIP wrapper class.
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
#include "UipEthernet.h"
#include "TcpServer.h"
extern "C"
{
#include "utility/uip-conf.h"
}
/**
 * @brief
 * @note
 * @param
 * @retval
 */
TcpServer::TcpServer() :
    _conns(1)
{}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
TcpClient* TcpServer::accept()
{
    TcpClient* result = NULL;

    UipEthernet::ethernet->tick();
    for (uip_userdata_t * data = &TcpClient::all_data[0]; data < &TcpClient::all_data[_conns]; data++) {
        if
        (
            data->packets_in[0] != NOBLOCK &&
            (
                ((data->state & UIP_CLIENT_CONNECTED) && uip_conns[data->state & UIP_CLIENT_SOCKETS].lport == _port) ||
                ((data->state & UIP_CLIENT_REMOTECLOSED) && ((uip_userdata_closed_t*)data)->lport == _port)
            )
        ) {
            data->ripaddr[0] =  uip_conns[data->state & UIP_CLIENT_SOCKETS].ripaddr[0];
            data->ripaddr[1] =  uip_conns[data->state & UIP_CLIENT_SOCKETS].ripaddr[1];
            result = new TcpClient(data);
            result->setInstance(result);
            return result;
        }
    }

    return result;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void TcpServer::open(UipEthernet* ethernet)
{
    if (UipEthernet::ethernet != ethernet)
        UipEthernet::ethernet = ethernet;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void TcpServer::bind(uint8_t port)
{
    _port = htons(port);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void TcpServer::bind(const char* ip, uint8_t port)
{
    _port = htons(port);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void TcpServer::listen(uint8_t conns)
{
    _conns = _conns < UIP_CONNS ? conns : UIP_CONNS;
    uip_listen(_port);
    UipEthernet::ethernet->tick();
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
size_t TcpServer::send(uint8_t c)
{
    return send(&c, 1);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
size_t TcpServer::send(const uint8_t* buf, size_t size)
{
    size_t  ret = 0;
    for (uip_userdata_t * data = &TcpClient::all_data[0]; data < &TcpClient::all_data[_conns]; data++) {
        if ((data->state & UIP_CLIENT_CONNECTED) && uip_conns[data->state & UIP_CLIENT_SOCKETS].lport == _port)
            ret += TcpClient::_write(data, buf, size);
    }

    return ret;
}

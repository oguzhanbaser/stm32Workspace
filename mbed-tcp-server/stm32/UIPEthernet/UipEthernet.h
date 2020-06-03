/*
 UipEthernet.h - Arduino implementation of a UIP wrapper class.
 Copyright (c) 2013 Norbert Truchsess <norbert.truchsess@t-online.de>
 All rights reserved.

 Modified (ported to mbed) by Zoltan Hudak <hudakz@inbox.com>

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
#ifndef UIPETHERNET_h
#define UIPETHERNET_h

//#define UIPETHERNET_DEBUG
//#define UIPETHERNET_DEBUG_CHKSUM
//#define UIPETHERNET_DEBUG_UDP
//#define UIPETHERNET_DEBUG_CLIENT
#include "mbed.h"
#include "DhcpClient.h"
#include "IpAddress.h"
#include "utility/Enc28j60Eth.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "UdpSocket.h"

extern "C"
{
#include "utility/uip_timer.h"
#include "utility/uip.h"
#include "utility/util.h"
}

class UipEthernet
{
public:
    static UipEthernet* ethernet;
    static IpAddress    dnsServerAddress;
    Enc28j60Eth         enc28j60Eth;

    UipEthernet (const uint8_t mac[6], PinName mosi, PinName miso, PinName sck, PinName cs);

    int               connect(unsigned long timeout = 60);
    void              disconnect();
    void              set_network(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4);
    void              set_network(IpAddress ip);
    void              set_network(IpAddress ip, IpAddress dns);
    void              set_network(IpAddress ip, IpAddress dns, IpAddress gateway);
    void              set_network(IpAddress ip, IpAddress dns, IpAddress gateway, IpAddress subnet);
    void              set_network(const char *ip_address, const char *netmask, const char *gateway);
    void              tick();
    IpAddress         localIP();
    IpAddress         subnetMask();
    IpAddress         gatewayIP();
    IpAddress         dnsServerIP();
    const char*       get_ip_address();
    const char*       get_netmask();
    const char*       get_gateway();
    static uint16_t   chksum(uint16_t sum, const uint8_t* data, uint16_t len);
    static uint16_t   ipchksum();
    bool              stoip4(const char *ip4addr, size_t len, void *dest);
private:
    uint8_t *const    _mac;
    IpAddress         _ip;
    IpAddress         _dns;
    IpAddress         _gateway;
    IpAddress         _subnet;
    static memhandle  inPacket;
    static memhandle  uipPacket;
    static uint8_t    uipHeaderLen;
    static uint8_t    packetState;
    DhcpClient        dhcpClient;
    Timer             periodicTimer;
    void              init(const uint8_t* mac);
    bool              network_send();
    friend class      TcpServer;
    friend class      TcpClient;
    friend class      UdpSocket;
#if UIP_UDP
    uint16_t          upper_layer_chksum(uint8_t proto);
#endif
    friend uint16_t   uip_ipchksum();
    friend uint16_t   uip_tcpchksum();
    friend uint16_t   uip_udpchksum();
    friend void       uipclient_appcall();
    friend void       uipudp_appcall();

#if UIP_CONF_IPV6
    uint16_t          uip_icmp6chksum();
#endif
};

#endif

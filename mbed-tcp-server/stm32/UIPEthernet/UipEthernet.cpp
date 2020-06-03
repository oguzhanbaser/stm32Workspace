/*
 UIPEthernet.cpp - Arduino implementation of a UIP wrapper class.
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
#include "mbed.h"
#include "UipEthernet.h"
#include "utility/Enc28j60Eth.h"
#include "UdpSocket.h"

extern "C"
{
#include "utility/uip-conf.h"
#include "utility/uip.h"
#include "utility/uip_arp.h"
#include "utility/uip_timer.h"
}
#define ETH_HDR ((struct uip_eth_hdr*) &uip_buf[0])

UipEthernet* UipEthernet::  ethernet = NULL;
memhandle UipEthernet::     inPacket(NOBLOCK);
memhandle UipEthernet::     uipPacket(NOBLOCK);
uint8_t UipEthernet::       uipHeaderLen(0);
uint8_t UipEthernet::       packetState(0);
IpAddress UipEthernet::     dnsServerAddress;

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void enc28j60_mempool_block_move_callback(memaddress dest, memaddress src, memaddress len)
{
    //as ENC28J60 DMA is unable to copy single byte:
    if (len == 1) {
        UipEthernet::ethernet->enc28j60Eth.writeByte(dest, UipEthernet::ethernet->enc28j60Eth.readByte(src));
    }
    else {
        // calculate address of last byte
        len += src - 1;

        /* 1. Appropriately program the EDMAST, EDMAND
              and EDMADST register pairs. The EDMAST
              registers should point to the first byte to copy
              from, the EDMAND registers should point to the
              last byte to copy and the EDMADST registers
              should point to the first byte in the destination
              range. The destination range will always be
              linear, never wrapping at any values except from
              8191 to 0 (the 8-Kbyte memory boundary).
              Extreme care should be taken when
              programming the start and end pointers to
              prevent a never ending DMA operation which
              would overwrite the entire 8-Kbyte buffer.
       */
        UipEthernet::ethernet->enc28j60Eth.writeRegPair(EDMASTL, src);
        UipEthernet::ethernet->enc28j60Eth.writeRegPair(EDMADSTL, dest);

        if ((src <= RXEND_INIT) && (len > RXEND_INIT))
            len -= ((RXEND_INIT + 1) - RXSTART_INIT);
        UipEthernet::ethernet->enc28j60Eth.writeRegPair(EDMANDL, len);

        /* 2. If an interrupt at the end of the copy process is
              desired, set EIE.DMAIE and EIE.INTIE and
              clear EIR.DMAIF.

           3. Verify that ECON1.CSUMEN is clear. */
        UipEthernet::ethernet->enc28j60Eth.writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_CSUMEN);

        /* 4. Start the DMA copy by setting ECON1.DMAST. */
        UipEthernet::ethernet->enc28j60Eth.writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_DMAST);

        //    wait until runnig DMA is completed
        while (UipEthernet::ethernet->enc28j60Eth.readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_DMAST);
    }
}

/*
 * Because UIP isn't encapsulated within a class we have to use global
 * variables, so we can only have one TCP/IP stack per program.
 */
UipEthernet::UipEthernet(const uint8_t mac[6], PinName mosi, PinName miso, PinName sck, PinName cs) :
    enc28j60Eth(mosi, miso, sck, cs),
    _mac(new uint8_t[6]),
    _ip(),
    _dns(),
    _gateway(),
    _subnet()
{
    for (uint8_t i = 0; i < 6; i++)
        _mac[i] = mac[i];
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
int UipEthernet::connect(unsigned long timeout)
{
    // Inicialize static pointer to the UIPEthernet instance
    ethernet = this;

    // Initialise the basic info
    init(_mac);

    // If no local IP address has been set ask DHCP server to provide one
    if (_ip == IpAddress()) {
        // Now try to get our config info from a DHCP server
        int ret = dhcpClient.begin((uint8_t*)_mac, timeout);

        if (ret == 1) {
            // We've successfully found a DHCP server and got our configuration info, so set things
            // accordingly
            set_network
            (
                dhcpClient.getLocalIp(),
                dhcpClient.getDnsServerIp(),
                dhcpClient.getGatewayIp(),
                dhcpClient.getSubnetMask()
            );

            return 0;
        }

        return -1;
    }
    else {
        return 0;
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::disconnect()
{
     ethernet = NULL;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(const char *ip_address, const char *netmask, const char *gateway)
{
    _ip = IpAddress(ip_address, strlen(ip_address));
    _dns = _ip;
    _dns[3] = 1;
    _gateway = IpAddress(gateway, strlen(gateway));
    _subnet = IpAddress(netmask, strlen(netmask));
    set_network(_ip, _dns, _gateway, _subnet);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4)
{
    _ip = IpAddress(octet1, octet2, octet3, octet4);
    set_network(_ip);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(IpAddress ip)
{
    _ip = ip;
    _dns = ip;
    _dns[3] = 1;
    set_network(ip, dns);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(IpAddress ip, IpAddress dns)
{
    _ip = ip;
    _dns = dns;
    _gateway = ip;
    _gateway[3] = 1;
    set_network(ip, dns, _gateway);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(IpAddress ip, IpAddress dns, IpAddress gateway)
{
    _ip = ip;
    _dns = dns;
    _gateway = gateway;
    _subnet = IpAddress(255, 255, 255, 0);
    set_network(_ip, _dns, _gateway, _subnet);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::set_network(IpAddress ip, IpAddress dns, IpAddress gateway, IpAddress subnet)
{
    uip_ipaddr_t    ipaddr;

    uip_ip_addr(ipaddr, ip);
    uip_sethostaddr(ipaddr);

    uip_ip_addr(ipaddr, gateway);
    uip_setdraddr(ipaddr);

    uip_ip_addr(ipaddr, subnet);
    uip_setnetmask(ipaddr);

    dnsServerAddress = dns;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress UipEthernet::localIP()
{
    uip_ipaddr_t    a;

    uip_gethostaddr(a);
    return ip_addr_uip(a);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress UipEthernet::subnetMask()
{
    uip_ipaddr_t    a;

    uip_getnetmask(a);
    return ip_addr_uip(a);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress UipEthernet::gatewayIP()
{
    uip_ipaddr_t    a;

    uip_getdraddr(a);
    return ip_addr_uip(a);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress UipEthernet::dnsServerIP()
{
    return dnsServerAddress;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
const char* UipEthernet::get_ip_address()
{
    static char buf[16];
    return localIP().toString(buf);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
const char* UipEthernet::get_netmask()
{
    static char buf[16];
    return subnetMask().toString(buf);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
const char* UipEthernet::get_gateway()
{
    static char buf[16];
    return gatewayIP().toString(buf);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::tick()
{
    if (inPacket == NOBLOCK) {
        inPacket = enc28j60Eth.receivePacket();
#ifdef UIPETHERNET_DEBUG
        if (inPacket != NOBLOCK) {
            printf("--------------\r\nreceivePacket: %d\r\n", inPacket);
        }
#endif
    }

    if (inPacket != NOBLOCK) {
        packetState = UIPETHERNET_FREEPACKET;
        uip_len = enc28j60Eth.blockSize(inPacket);
        if (uip_len > 0) {
            enc28j60Eth.readPacket(inPacket, 0, (uint8_t*)uip_buf, UIP_BUFSIZE);
            if (ETH_HDR->type == HTONS(UIP_ETHTYPE_IP)) {
                uipPacket = inPacket;   //required for upper_layer_checksum of in_packet!
#ifdef UIPETHERNET_DEBUG
                printf("readPacket type IP, uip_len: %d\r\n", uip_len);
#endif
                uip_arp_ipin();
                uip_input();
                if (uip_len > 0) {
                    uip_arp_out();
                    network_send();
                }
            }
            else
            if (ETH_HDR->type == HTONS(UIP_ETHTYPE_ARP))
            {
#ifdef UIPETHERNET_DEBUG
                printf("readPacket type ARP, uip_len: %d\r\n", uip_len);
#endif
                uip_arp_arpin();
                if (uip_len > 0) {
                    network_send();
                }
            }
        }

        if (inPacket != NOBLOCK && (packetState & UIPETHERNET_FREEPACKET))
        {
#ifdef UIPETHERNET_DEBUG
            printf("freeing packet: %d\r\n", inPacket);
#endif
            enc28j60Eth.freePacket();
            inPacket = NOBLOCK;
        }
    }

    bool    periodic = periodicTimer.read_ms() > UIP_PERIODIC_TIMEOUT;

    for (int i = 0; i < UIP_CONNS; i++) {
        uip_conn = &uip_conns[i];
        if (periodic) {
            uip_process(UIP_TIMER);
        }
        else {
            uip_userdata_t*     data = (uip_userdata_t*)uip_conn->appstate;
            if (data != NULL) {
                if (data->pollTimer.read_ms() >= UIP_CLIENT_TIMEOUT) {
                    uip_process(UIP_POLL_REQUEST);
                    data->pollTimer.stop();
                    data->pollTimer.reset();
                }
            }
            else
                continue;
        }

        // If the above function invocation resulted in data that
        // should be sent out on the Enc28J60Network, the global variable
        // uip_len is set to a value > 0.
        if (uip_len > 0) {
            uip_arp_out();
            network_send();
        }
    }

    if (periodic) {
        periodicTimer.reset();
#if UIP_UDP
        for (int i = 0; i < UIP_UDP_CONNS; i++) {
            uip_udp_periodic(i);

            // If the above function invocation resulted in data that
            // should be sent out on the Enc28J60Network, the global variable
            // uip_len is set to a value > 0.
            if (uip_len > 0) {
                UdpSocket::_send((uip_udp_userdata_t *) (uip_udp_conns[i].appstate));
            }
        }
#endif // UIP_UDP
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
bool UipEthernet::network_send()
{
    if (packetState & UIPETHERNET_SENDPACKET)
    {
#ifdef UIPETHERNET_DEBUG
        printf("Enc28J60Network_send uipPacket: %d, hdrlen: %d\r\n", inPacket, uipHeaderLen);
#endif
        enc28j60Eth.writePacket(uipPacket, 0, uip_buf, uipHeaderLen);
        packetState &= ~UIPETHERNET_SENDPACKET;
        goto sendandfree;
    }

    uipPacket = Enc28j60Eth::allocBlock(uip_len);
    if (uipPacket != NOBLOCK)
    {
#ifdef UIPETHERNET_DEBUG
        printf("Enc28J60Network_send uip_buf (uip_len): %d, packet: %d\r\n", uip_len, uipPacket);
#endif
        enc28j60Eth.writePacket(uipPacket, 0, uip_buf, uip_len);
        goto sendandfree;
    }

#ifdef UIPETHERNET_DEBUG
        printf("Enc28J60Network_send return false\r\n");
#endif
    return false;
sendandfree:
    enc28j60Eth.sendPacket(uipPacket);
    Enc28j60Eth::freeBlock(uipPacket);
    uipPacket = NOBLOCK;
    return true;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UipEthernet::init(const uint8_t* mac)
{
    periodicTimer.start();

    enc28j60Eth.init((uint8_t*)mac);
    uip_seteth_addr(mac);

    uip_init();
    uip_arp_init();
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
//void UIPEthernet::configure(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
//{
//    uip_ipaddr_t    ipaddr;
//    uip_ip_addr(ipaddr, ip);
//    uip_sethostaddr(ipaddr);
//    uip_ip_addr(ipaddr, gateway);
//    uip_setdraddr(ipaddr);
//    uip_ip_addr(ipaddr, subnet);
//    uip_setnetmask(ipaddr);
//    dnsServerAddress = dns;
//}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint16_t UipEthernet::chksum(uint16_t sum, const uint8_t* data, uint16_t len)
{
    uint16_t        t;
    const uint8_t*  dataptr;
    const uint8_t*  last_byte;

    dataptr = data;
    last_byte = data + len - 1;

    while (dataptr < last_byte) {
        /* At least two more bytes */
        t = (dataptr[0] << 8) + dataptr[1];
        sum += t;
        if (sum < t) {
            sum++;  /* carry */
        }

        dataptr += 2;
    }

    if (dataptr == last_byte) {
        t = (dataptr[0] << 8) + 0;
        sum += t;
        if (sum < t) {
            sum++;  /* carry */
        }
    }

    /* Return sum in host byte order. */
    return sum;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint16_t UipEthernet::ipchksum()
{
    uint16_t    sum;

    sum = chksum(0, &uip_buf[UIP_LLH_LEN], UIP_IPH_LEN);
    return(sum == 0) ? 0xffff : htons(sum);
}

uint16_t
#if UIP_UDP
UipEthernet::upper_layer_chksum(uint8_t proto)
#else
uip_tcpchksum()
#endif
{
    uint16_t    upper_layer_len;
    uint16_t    sum;

#if UIP_CONF_IPV6
    upper_layer_len = (((u16_t) (BUF->len[0]) << 8) + BUF->len[1]);
#else /* UIP_CONF_IPV6 */
    upper_layer_len = (((u16_t) (BUF->len[0]) << 8) + BUF->len[1]) - UIP_IPH_LEN;
#endif /* UIP_CONF_IPV6 */

    /* First sum pseudoheader. */

    /* IP protocol and length fields. This addition cannot carry. */
#if UIP_UDP
    sum = upper_layer_len + proto;
#else
    sum = upper_layer_len + UIP_PROTO_TCP;
#endif
    /* Sum IP source and destination addresses. */

    sum = UipEthernet::chksum(sum, (u8_t*) &BUF->srcipaddr[0], 2 * sizeof(uip_ipaddr_t));

    uint8_t upper_layer_memlen;
#if UIP_UDP
    switch (proto) {
        //    case UIP_PROTO_ICMP:
        //    case UIP_PROTO_ICMP6:
        //      upper_layer_memlen = upper_layer_len;
        //      break;
        case UIP_PROTO_UDP:
            upper_layer_memlen = UIP_UDPH_LEN;
            break;

        default:
            //  case UIP_PROTO_TCP:
    #endif
            upper_layer_memlen = (BUF->tcpoffset >> 4) << 2;
    #if UIP_UDP
            break;
    }
#endif
    sum = UipEthernet::chksum(sum, &uip_buf[UIP_IPH_LEN + UIP_LLH_LEN], upper_layer_memlen);
#ifdef UIPETHERNET_DEBUG_CHKSUM
    printf
        (
            "chksum uip_buf[%d-%d]: %d\r\n", UIP_IPH_LEN +
            UIP_LLH_LEN, UIP_IPH_LEN +
            UIP_LLH_LEN +
            upper_layer_memlen, htons(sum)
        );
#endif
    if (upper_layer_memlen < upper_layer_len) {
        sum = enc28j60Eth.chksum
            (
                sum, UipEthernet::uipPacket, UIP_IPH_LEN +
                UIP_LLH_LEN +
                upper_layer_memlen, upper_layer_len -
                upper_layer_memlen
            );
#ifdef UIPETHERNET_DEBUG_CHKSUM
        printf
            (
                "chksum uipPacket(%d)[%d-%d]: %d\r\n", uipPacket, UIP_IPH_LEN +
                UIP_LLH_LEN +
                upper_layer_memlen, UIP_IPH_LEN +
                UIP_LLH_LEN +
                upper_layer_len, htons(sum)
            );
#endif
    }
    return(sum == 0) ? 0xffff : htons(sum);
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint16_t uip_ipchksum()
{
    return UipEthernet::ethernet->ipchksum();
}

#if UIP_UDP

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint16_t uip_tcpchksum()
{
    uint16_t    sum = UipEthernet::ethernet->upper_layer_chksum(UIP_PROTO_TCP);

    return sum;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
uint16_t uip_udpchksum()
{
    uint16_t    sum = UipEthernet::ethernet->upper_layer_chksum(UIP_PROTO_UDP);

    return sum;
}

#endif

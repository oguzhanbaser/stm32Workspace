/*
 UIPUdp.cpp - Arduino implementation of a UIP wrapper class.
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
#include "UdpSocket.h"
#include "DnsClient.h"

extern "C"
{
#include "utility/uip-conf.h"
#include "utility/uip.h"
#include "utility/uip_arp.h"
}
#if UIP_UDP
#define UIP_ARPHDRSIZE  42
#define UDPBUF          ((struct uip_udpip_hdr*) &uip_buf[UIP_LLH_LEN])

/**
 * @brief
 * @note
 * @param
 * @retval
 */
UdpSocket::UdpSocket() :
    _uip_udp_conn(NULL),
    _timeout_ms(1000)
{
    memset(&appdata, 0, sizeof(appdata));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
UdpSocket::UdpSocket(int timeout_ms) :
    _uip_udp_conn(NULL),
    _timeout_ms(timeout_ms)
{
    memset(&appdata, 0, sizeof(appdata));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
UdpSocket::UdpSocket(UipEthernet* net, int timeout_ms /*= 1000*/ ) :
    _uip_udp_conn(NULL),
    _timeout_ms(timeout_ms)
{
    memset(&appdata, 0, sizeof(appdata));
    if (UipEthernet::ethernet != net)
        UipEthernet::ethernet = net;
}

// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
uint8_t UdpSocket::begin(uint16_t port)
{
    if (!_uip_udp_conn) {
        _uip_udp_conn = uip_udp_new(NULL, 0);
    }

    if (_uip_udp_conn) {
        uip_udp_bind(_uip_udp_conn, htons(port));
        _uip_udp_conn->appstate = &appdata;
        return 1;
    }

    return 0;
}

// Finish with the UDP socket
void UdpSocket::stop()
{
    if (_uip_udp_conn) {
        uip_udp_remove(_uip_udp_conn);
        _uip_udp_conn->appstate = NULL;
        _uip_udp_conn = NULL;
        UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_in);
        UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_next);
        UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_out);
        memset(&appdata, 0, sizeof(appdata));
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UdpSocket::close()
{
    stop();
}

// Sending UDP packets
// Start building up a packet to send to the remote host specific in ip and port

// Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
int UdpSocket::beginPacket(IpAddress ip, uint16_t port)
{
    UipEthernet::ethernet->tick();
    if (ip && port) {
        uip_ipaddr_t    ripaddr;
        uip_ip_addr(&ripaddr, ip);
#ifdef UIPETHERNET_DEBUG_UDP
        printf("udp beginPacket, ");
#endif
        if (_uip_udp_conn) {
            _uip_udp_conn->rport = htons(port);
            uip_ipaddr_copy(_uip_udp_conn->ripaddr, &ripaddr);
        }
        else {
            _uip_udp_conn = uip_udp_new(&ripaddr, htons(port));
            if (_uip_udp_conn)
            {
#ifdef UIPETHERNET_DEBUG_UDP
                printf("new connection, ");
#endif
                _uip_udp_conn->appstate = &appdata;
            }
            else
            {
#ifdef UIPETHERNET_DEBUG_UDP
                printf("failed to allocate new connection\r\n");
#endif
                return 0;
            }
        }

#ifdef UIPETHERNET_DEBUG_UDP
        char buf[16];
        printf("rip: %s, port: %d\r\n", ip.toString(buf), port);
#endif
    }

    if (_uip_udp_conn) {
        if (appdata.packet_out == NOBLOCK) {
            appdata.packet_out = UipEthernet::ethernet->enc28j60Eth.allocBlock(UIP_UDP_MAXPACKETSIZE);
            appdata.out_pos = UIP_UDP_PHYH_LEN;
            if (appdata.packet_out != NOBLOCK) {
                return 1;
            }
#ifdef UIPETHERNET_DEBUG_UDP
            else {
                printf("failed to allocate memory for packet\r\n");
            }
#endif
        }

#ifdef UIPETHERNET_DEBUG_UDP
        else {
            printf("previous packet on that connection not sent yet\r\n");
        }
#endif
    }

    return 0;
}

// Start building up a packet to send to the remote host specific in host and port

// Returns 1 if successful, 0 if there was a problem resolving the hostname or port
int UdpSocket::beginPacket(const char* host, uint16_t port)
{
    // Look up the host first
    int         ret = 0;
    DnsClient   dns;
    IpAddress   remote_addr;

    dns.begin(UipEthernet::ethernet->dnsServerIP());
    ret = dns.getHostByName(host, remote_addr);
    if (ret == 1) {
        return beginPacket(remote_addr, port);
    }
    else {
        return ret;
    }
}

// Finish off this packet and send it

// Returns 1 if the packet was sent successfully, 0 if there was an error
int UdpSocket::endPacket()
{
    if (_uip_udp_conn && appdata.packet_out != NOBLOCK) {
        appdata.send = true;
        UipEthernet::ethernet->enc28j60Eth.resizeBlock(appdata.packet_out, 0, appdata.out_pos);
        uip_udp_periodic_conn(_uip_udp_conn);
        if (uip_len > 0) {
            _send(&appdata);
            return 1;
        }
    }

    return 0;
}

// Write a single byte into the packet
size_t UdpSocket::write(uint8_t c)
{
    return write(&c, 1);
}

// Write size bytes from buffer into the packet
size_t UdpSocket::write(const uint8_t* buffer, size_t size)
{
    if (appdata.packet_out != NOBLOCK) {
        size_t  ret = UipEthernet::ethernet->enc28j60Eth.writePacket
            (
                appdata.packet_out,
                appdata.out_pos,
                (uint8_t*)buffer,
                size
            );
        appdata.out_pos += ret;
        return ret;
    }

    return 0;
}

// Start processing the next available incoming packet

// Returns the size of the packet in bytes, or 0 if no packets are available
int UdpSocket::parsePacket()
{
    UipEthernet::ethernet->tick();
#ifdef UIPETHERNET_DEBUG_UDP
    if (appdata.packet_in != NOBLOCK) {
        printf("udp parsePacket freeing previous packet: %d\r\n", appdata.packet_in);
    }
#endif
    UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_in);

    appdata.packet_in = appdata.packet_next;
    appdata.packet_next = NOBLOCK;

#ifdef UIPETHERNET_DEBUG_UDP
    if (appdata.packet_in != NOBLOCK) {
        printf("udp parsePacket received packet: %d", appdata.packet_in);
    }
#endif

    int size = UipEthernet::ethernet->enc28j60Eth.blockSize(appdata.packet_in);
#ifdef UIPETHERNET_DEBUG_UDP
    if (appdata.packet_in != NOBLOCK) {
        printf(", size: %d\r\n", size);
    }
#endif
    return size;
}

// Number of bytes remaining in the current packet
size_t UdpSocket::available()
{
    UipEthernet::ethernet->tick();
    return UipEthernet::ethernet->enc28j60Eth.blockSize(appdata.packet_in);
}

// Read a single byte from the current packet. Returns -1 if no byte is available.
int UdpSocket::read()
{
    static unsigned char    c;
    if (read(&c, 1) > 0) {
        return c;
    }

    return -1;
}

// Read up to len bytes from the current packet and place them into buffer

// Returns the number of bytes read, or 0 if none are available
size_t UdpSocket::read(unsigned char* buffer, size_t len)
{
    UipEthernet::ethernet->tick();
    if (appdata.packet_in != NOBLOCK) {
        memaddress  read = UipEthernet::ethernet->enc28j60Eth.readPacket(appdata.packet_in, 0, buffer, len);
        if (read == UipEthernet::ethernet->enc28j60Eth.blockSize(appdata.packet_in)) {
            UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_in);
            appdata.packet_in = NOBLOCK;
        }
        else
            UipEthernet::ethernet->enc28j60Eth.resizeBlock(appdata.packet_in, read);
        return read;
    }

    return 0;
}

// Return the next byte from the current packet without moving on to the next byte
int UdpSocket::peek()
{
    UipEthernet::ethernet->tick();
    if (appdata.packet_in != NOBLOCK) {
        unsigned char   c;
        if (UipEthernet::ethernet->enc28j60Eth.readPacket(appdata.packet_in, 0, &c, 1) == 1)
            return c;
    }

    return -1;
}

// Finish reading the current packet
void UdpSocket::flush()
{
    UipEthernet::ethernet->tick();
    UipEthernet::ethernet->enc28j60Eth.freeBlock(appdata.packet_in);
    appdata.packet_in = NOBLOCK;
}

// Return the IP address of the host who sent the current incoming packet
IpAddress UdpSocket::remoteIP()
{
    return _uip_udp_conn ? ip_addr_uip(_uip_udp_conn->ripaddr) : IpAddress();
}

// Return the port of the host who sent the current incoming packet
uint16_t UdpSocket::remotePort()
{
    return _uip_udp_conn ? ntohs(_uip_udp_conn->rport) : 0;
}

// UIP callback function
void uipudp_appcall()
{
    if (uip_udp_userdata_t * data = (uip_udp_userdata_t *) (uip_udp_conn->appstate)) {
        if (uip_newdata()) {
            if (data->packet_next == NOBLOCK) {
                uip_udp_conn->rport = UDPBUF->srcport;
                uip_ipaddr_copy(uip_udp_conn->ripaddr, UDPBUF->srcipaddr);
                data->packet_next = UipEthernet::ethernet->enc28j60Eth.allocBlock(ntohs(UDPBUF->udplen) - UIP_UDPH_LEN);

                //if we are unable to allocate memory the packet is dropped. udp doesn't guarantee packet delivery
                if (data->packet_next != NOBLOCK) {
                    //discard Linklevel and IP and udp-header and any trailing bytes:
                    UipEthernet::ethernet->enc28j60Eth.copyPacket
                        (
                            data->packet_next,
                            0,
                            UipEthernet::inPacket,
                            UIP_UDP_PHYH_LEN,
                            UipEthernet::ethernet->enc28j60Eth.blockSize(data->packet_next)
                        );
#ifdef UIPETHERNET_DEBUG_UDP
                    printf
                    (
                        "udp, uip_newdata received packet: %d, size: %d\r\n",
                        data->packet_next,
                        UipEthernet::ethernet->enc28j60Eth.blockSize(data->packet_next)
                    );
#endif
                }
            }
        }

        if (uip_poll() && data->send)
        {
            //set uip_slen (uip private) by calling uip_udp_send
#ifdef UIPETHERNET_DEBUG_UDP
            printf
            (
                "udp, uip_poll preparing packet to send: %d, size: %d\r\n",
                data->packet_out,
                UipEthernet::ethernet->enc28j60Eth.blockSize(data->packet_out)
            );
#endif
            UipEthernet::uipPacket = data->packet_out;
            UipEthernet::uipHeaderLen = UIP_UDP_PHYH_LEN;
            uip_udp_send(data->out_pos - (UIP_UDP_PHYH_LEN));
        }
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UdpSocket::_send(uip_udp_userdata_t* data)
{
    uip_arp_out();  //add arp
    if (uip_len == UIP_ARPHDRSIZE) {
        UipEthernet::uipPacket = NOBLOCK;
        UipEthernet::packetState &= ~UIPETHERNET_SENDPACKET;
#ifdef UIPETHERNET_DEBUG_UDP
        printf("udp, uip_poll results in ARP-packet\r\n");
#endif
    }
    else {
        //arp found ethaddr for ip (otherwise packet is replaced by arp-request)
        data->send = false;
        data->packet_out = NOBLOCK;
        UipEthernet::packetState |= UIPETHERNET_SENDPACKET;
#ifdef UIPETHERNET_DEBUG_UDP
        printf("udp, uip_packet to send: %d\r\n", UipEthernet::uipPacket);
#endif
    }

    UipEthernet::ethernet->network_send();
}
#endif

/**
 * @brief
 * @note
 * @param
 * @retval
 */
nsapi_size_or_error_t UdpSocket::sendto(const char* host, uint16_t port, const void* data, size_t size)
{
    DnsClient   dns;
    IpAddress   address;
    uint32_t    address_bytes;

    dns.begin(UipEthernet::ethernet->dnsServerIP());
    if (dns.getHostByName(host, address) == 1) {
        address_bytes = address;
        _remote_addr = SocketAddress(&address_bytes, NSAPI_IPv4, port);
    }
    else {
        _remote_addr = SocketAddress(host, port);
    }

    if (beginPacket(host, port) == 0) {
        stop();
        return NSAPI_ERROR_NO_ADDRESS;
    }

    if (write((uint8_t*)data, size) == 0) {
        stop();
        return NSAPI_ERROR_WOULD_BLOCK;
    };

    if (endPacket() <= 0) {
        stop();
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    return NSAPI_ERROR_OK;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
nsapi_size_or_error_t UdpSocket::sendto(const SocketAddress& address, const void* data, size_t size)
{
    IpAddress   ip_addr(address.get_addr().bytes);
    uint16_t    port = address.get_port();

    _remote_addr = address;

    if (beginPacket(ip_addr, port) == 0) {
        stop();
        return NSAPI_ERROR_NO_ADDRESS;
    }

    if (write((uint8_t*)data, size) == 0) {
        stop();
        return NSAPI_ERROR_WOULD_BLOCK;
    };

    if (endPacket() <= 0) {
        stop();
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    return NSAPI_ERROR_OK;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
nsapi_size_or_error_t UdpSocket::recvfrom(SocketAddress* address, void* data, size_t size)
{
    *address = _remote_addr;

    Timer   timer;
    int     success;

    timer.start();
    do {
        success = parsePacket();
    } while (!success && (timer.read_ms() < _timeout_ms));

    if (!success) {
        stop();
        return NSAPI_ERROR_WOULD_BLOCK;
    }

    size_t      n;
    size_t      recv_count = 0;
    uint8_t*    pos = (uint8_t*)data;

    do {
        if (recv_count + available() <= size) {
            n = read(pos, available());
            pos += n;
            recv_count += n;
        }
        else {
            return NSAPI_ERROR_NO_MEMORY;
        }
    } while ((available() > 0) && (recv_count < size));

    flush();

    return NSAPI_ERROR_OK;
}

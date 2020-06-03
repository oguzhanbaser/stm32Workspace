/*
 UIPUdp.h - Arduino implementation of a UIP wrapper class.
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
#ifndef UDPSOCKET_h
#define UDPSOCKET_h

#include "mbed.h"
#include "SocketAddress.h"
#include "utility/Udp.h"
#include "IpAddress.h"
#include "utility/MemPool.h"
extern "C"
{
#include "utility/uip.h"
}
#define UIP_UDP_MAXDATALEN      1500
#define UIP_UDP_PHYH_LEN        UIP_LLH_LEN + UIP_IPUDPH_LEN
#define UIP_UDP_MAXPACKETSIZE   UIP_UDP_MAXDATALEN + UIP_UDP_PHYH_LEN

typedef struct
{
    memaddress  out_pos;
    memhandle   packet_next;
    memhandle   packet_in;
    memhandle   packet_out;
    bool        send;
} uip_udp_userdata_t;

class UipEthernet;

class UdpSocket :  public Udp
{
private:
    uip_udp_userdata_t      appdata;
    struct uip_udp_conn*    _uip_udp_conn;
    SocketAddress           _remote_addr;
    int                     _timeout_ms;
public:
    UdpSocket();                        // Constructor
    UdpSocket(int timeout_ms);          // Constructor
    UdpSocket(UipEthernet* ethernet, int timeout_ms = 1000);
    virtual     ~UdpSocket()    { }     // Virtual destructor
    uint8_t     begin(uint16_t port);   // initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
    void        stop();                 // Finish with the UDP socket
    void        close();                // Close the UDP socket
    // Sending UDP packets
    // Start building up a packet to send to the remote host specific in ip and port
    // Returns 1 if successful, 0 if there was a problem with the supplied IP address or port
    int         beginPacket(IpAddress ip, uint16_t port);

    // Start building up a packet to send to the remote host specific in host and port
    // Returns 1 if successful, 0 if there was a problem resolving the hostname or port
    int         beginPacket(const char* host, uint16_t port);

    // Finish off this packet and send it
    // Returns 1 if the packet was sent successfully, 0 if there was an error
    int         endPacket();

    // Write a single byte into the packet
    size_t      write(uint8_t);

    // Write size bytes from buffer into the packet
    size_t      write(const uint8_t* buffer, size_t size);

    //  using Print::write;
    // Start processing the next available incoming packet
    // Returns the size of the packet in bytes, or 0 if no packets are available
    int         parsePacket();

    // Number of bytes remaining in the current packet
    size_t      available();

    // Read a single byte from the current packet
    int         read();

    // Read up to len bytes from the current packet and place them into buffer
    // Returns the number of bytes read, or 0 if none are available
    size_t      read(unsigned char* buffer, size_t len);
    // Read up to len characters from the current packet and place them into buffer

    // Returns the number of characters read, or 0 if none are available
    size_t      read(char* buffer, size_t len)  { return read((unsigned char*)buffer, len); }

    // Return the next byte from the current packet without moving on to the next byte
    int         peek();
    void        flush();    // Finish reading the current packet

    // Return the IP address of the host who sent the current incoming packet
    IpAddress   remoteIP();

    // Return the port of the host who sent the current incoming packet
    uint16_t    remotePort();

    // Send data to the specified host and port.
    nsapi_size_or_error_t sendto (const char *host, uint16_t port, const void *data, size_t size);
    // Send data to the specified address.
    nsapi_size_or_error_t sendto (const SocketAddress &address, const void *data, size_t size);
    // Receive a datagram and store the source address in address if it's not NULL.
    nsapi_size_or_error_t recvfrom (SocketAddress *address, void *data, size_t size);

private:
    friend void     uipudp_appcall();

    friend class    UipEthernet;
    static void     _send(uip_udp_userdata_t* data);
};
#endif

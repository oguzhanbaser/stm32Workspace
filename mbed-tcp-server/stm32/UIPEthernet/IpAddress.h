/*
  IpAddress.h - Base class that provides IPAddress
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  Modified (ported to mbed) by Zoltan Hudak <hudakz@inbox.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef IPADDRESS_h
#define IPADDRESS_h

#include <stdio.h>

// A class to make it easier to handle and pass around IP addresses

class   IpAddress
{
private:
    uint8_t _address[4];    // IPv4 address

public:
    // Constructors
    IpAddress(void);
    IpAddress(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4);
    IpAddress(uint32_t address);
    IpAddress(const uint8_t address[4]);
    IpAddress(const char *str, size_t len);

    // Overloaded cast operator to allow IPAddress objects to be used where a pointer
    // to a four-byte uint8_t array is expected
    operator uint32_t(void) const    { return *((uint32_t*)_address); }
    bool operator==(const IpAddress& addr) const { return(*((uint32_t*)_address)) == (*((uint32_t*)addr._address)); }
    bool operator==(const uint8_t* addr) const;

    // Overloaded index operator to allow getting and setting individual octets of the address
    uint8_t operator[](int index) const { return _address[index]; }
    uint8_t &operator[](int index)      { return _address[index]; }

    // Overloaded copy operators to allow initialisation of IPAddress objects from other types
    IpAddress &operator =(uint32_t address);
    IpAddress &operator =(const uint8_t* address);

    // Returns IP Address as string of char
    const char* toString(char* buf);

    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
    uint8_t*    rawAddress(void)   { return _address; }

    friend class UIPEthernet;
    friend class UdpSocket;
    friend class TcpClient;
    friend class TcpServer;
    friend class DhcpClient;
    friend class DnsClient;
};

const IpAddress INADDR_NONE(0, 0, 0, 0);
#endif

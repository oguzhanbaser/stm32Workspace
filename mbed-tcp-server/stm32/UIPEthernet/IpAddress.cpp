/*
  IPAddress.cpp - Base class that provides IPAddress
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
#include <stdio.h>
#include "mbed.h"
#include "IpAddress.h"

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress::IpAddress(void)
{
    memset(_address, 0, sizeof(_address));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress::IpAddress(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4)
{
    _address[0] = octet1;
    _address[1] = octet2;
    _address[2] = octet3;
    _address[3] = octet4;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress::IpAddress(uint32_t address)
{
    memcpy(_address, &address, sizeof(_address));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress::IpAddress(const uint8_t address[4])
{
    memcpy(_address, address, sizeof(_address));
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress::IpAddress(const char* str, size_t len)
{
    uint8_t pos = 0;
    uint8_t byte;
    uint8_t i = 0;

    if (len > 16)
        return;

    while (true) {
        if (pos == len || str[pos] < '0' || str[pos] > '9') {
            return;
        }

        byte = 0;
        while (pos < len && str[pos] >= '0' && str[pos] <= '9') {
            byte *= 10;
            byte += str[pos++] - '0';
        }

        _address[i++] = byte;

        if (i == 4) {
            return;
        }

        if (pos == len || str[pos++] != '.') {
            return;
        }
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress &IpAddress::operator=(const uint8_t* address) {
    memcpy(_address, address, sizeof(_address));
    return *this;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
IpAddress &IpAddress::operator=(uint32_t address)
{
    memcpy(_address, (const uint8_t*) &address, sizeof(_address));
    return *this;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
bool IpAddress::operator==(const uint8_t* addr) const
{
    return memcmp(addr, _address, sizeof(_address)) == 0;
}

/**
 * @brief   Returns IP Address as string of char
 * @note
 * @param
 * @retval
 */
const char* IpAddress::toString(char* buf)
{
    uint8_t i = 0;
    uint8_t j = 0;

    for (i = 0; i < 3; i++) {
        j += sprintf(&buf[j], "%d", _address[i]);
        buf[j++] = '.';
    }

    j += sprintf(&buf[j], "%d", _address[i]);
    buf[j] = '\0';
    return buf;
}

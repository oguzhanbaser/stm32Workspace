/*
 clock-arch.c - mbed implementation of UIP clock device.
 Copyright (c) 2010 Adam Nielsen <malvineous@shikadi.net>
 All rights reserved.

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
#include <time.h>
#include "clock-arch.h"

/**
 * @brief
 * @note
 * @param
 * @retval
 */
clock_time_t clock_time(void) {
    return(clock_time_t) time(NULL);
}

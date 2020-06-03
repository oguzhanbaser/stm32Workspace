/*
 Enc28J60Network.h
 UIPEthernet network driver for Microchip ENC28J60 Ethernet Interface.

 Copyright (c) 2013 Norbert Truchsess <norbert.truchsess@t-online.de>
 All rights reserved.

 based on enc28j60.c file from the AVRlib library by Pascal Stang.
 For AVRlib See http://www.procyonengineering.com/

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
#ifndef ENC28J60PHY_H
#define ENC28J60PHY_H

#include "mbed.h"
#include "MemPool.h"

#define UIP_RECEIVEBUFFERHANDLE 0xff

//#define ENC28J60DEBUG

class Enc28j60Eth : public MemPool
{
private:
    SPI             _spi;
    DigitalOut      _cs;
    static uint16_t nextPacketPtr;
    static uint8_t  bank;

    static struct memblock  receivePkt;

    uint16_t    setReadPtr(memhandle handle, memaddress position, uint16_t len);
    void        setERXRDPT();
    void        readBuffer(uint16_t len, uint8_t* data);
    void        writeBuffer(uint16_t len, uint8_t* data);
    void        setBank(uint8_t address);
    uint8_t     readReg(uint8_t address);
    void        writeReg(uint8_t address, uint8_t data);
    void        phyWrite(uint8_t address, uint16_t data);
    uint16_t    phyRead(uint8_t address);
    void        clkout(uint8_t clk);

    friend void enc28j60_mempool_block_move_callback(memaddress, memaddress, memaddress);
public:
    Enc28j60Eth(PinName mosi, PinName miso, PinName sclk, PinName cs);
    uint8_t     getrev();
    void        powerOn();
    void        powerOff();
    bool        linkStatus();

    void        init(uint8_t* macaddr);
    memhandle   receivePacket();
    void        freePacket();
    size_t      blockSize(memhandle handle);
    void        sendPacket(memhandle handle);
    uint16_t    readPacket(memhandle handle, memaddress position, uint8_t* buffer, uint16_t len);
    uint16_t    writePacket(memhandle handle, memaddress position, uint8_t* buffer, uint16_t len);
    void        copyPacket(memhandle dest, memaddress dest_pos, memhandle src, memaddress src_pos, uint16_t len);
    uint16_t    chksum(uint16_t sum, memhandle handle, memaddress pos, uint16_t len);

    uint8_t     readOp(uint8_t op, uint8_t address);
    uint8_t     readByte(uint16_t addr);
    void        writeOp(uint8_t op, uint8_t address, uint8_t data);
    void        writeRegPair(uint8_t address, uint16_t data);
    void        writeByte(uint16_t addr, uint8_t data);
};

#endif

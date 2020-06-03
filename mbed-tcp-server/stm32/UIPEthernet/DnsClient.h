// Arduino DNS client for Enc28J60-based Ethernet shield
// (c) Copyright 2009-2010 MCQN Ltd.
// Released under Apache License, version 2.0
#ifndef DNSClient_h
#define DNSClient_h

#include "UdpSocket.h"
#include "IpAddress.h"

class   DnsClient
{
public:
    // ctor
    void        begin(const IpAddress& aDNSServer);

    /** Convert a numeric IP address string into a four-byte IP address.
        @param aIPAddrString IP address to convert
        @param aResult IPAddress structure to store the returned IP address
        @result 1 if aIPAddrString was successfully converted to an IP address,
                else error code
    */
    int         inet_aton(const char* aIPAddrString, IpAddress& aResult);

    /** Resolve the given hostname to an IP address.
        @param aHostname Name to be resolved
        @param aResult IPAddress structure to store the returned IP address
        @result 1 if aIPAddrString was successfully converted to an IP address,
                else error code
    */
    int         getHostByName(const char* aHostname, IpAddress& aResult);
protected:
    uint16_t    buildRequest(const char* aName);
    int16_t     processResponse(uint16_t aTimeout, IpAddress& aAddress);

    IpAddress   iDNSServer;
    uint16_t    iRequestId;
    UdpSocket   iUdp;
};
#endif

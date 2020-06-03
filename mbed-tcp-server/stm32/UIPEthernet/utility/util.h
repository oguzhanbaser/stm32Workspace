#ifndef UTIL_H
#define UTIL_H

#define htons(x)    (u16_t) ((((u16_t) (x)) << 8) | (((u16_t) (x)) >> 8))
//#define ntohs(x)    htons(x)
#define htonl(x)    (((x) << 24 & 0xFF000000UL) | ((x) << 8 & 0x00FF0000UL) | ((x) >> 8 & 0x0000FF00UL) | ((x) >> 24 & 0x000000FFUL))
#define ntohl(x)    htonl(x)
#define UIPETHERNET_FREEPACKET  1
#define UIPETHERNET_SENDPACKET  2

#define uip_ip_addr(addr, ip) \
    do { \
        ((u16_t *) (addr))[0] = (((ip[1]) << 8) | (ip[0])); \
        ((u16_t *) (addr))[1] = (((ip[3]) << 8) | (ip[2])); \
    } while (0)
#define ip_addr_uip(a)  IpAddress(a[0] & 0xFF, a[0] >> 8, a[1] & 0xFF, a[1] >> 8)   //TODO this is not IPV6 capable

#define uip_seteth_addr(eaddr) \
    do { \
        uip_ethaddr.addr[0] = eaddr[0]; \
        uip_ethaddr.addr[1] = eaddr[1]; \
        uip_ethaddr.addr[2] = eaddr[2]; \
        uip_ethaddr.addr[3] = eaddr[3]; \
        uip_ethaddr.addr[4] = eaddr[4]; \
        uip_ethaddr.addr[5] = eaddr[5]; \
    } while (0)
#define BUF ((struct uip_tcpip_hdr*) &uip_buf[UIP_LLH_LEN])

#endif

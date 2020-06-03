//#define UIPDEBUG

#if defined(UIPDEBUG)
#include <inttypes.h>
#include "mbed.h"
#include "utility/uip_debug.h"
extern "C"
{
#include "uip.h"
}
struct uip_conn con[UIP_CONNS];

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UIPDebug::uip_debug_printconns(void) {
    for (uint8_t i = 0; i < UIP_CONNS; i++) {
        if (uip_debug_printcon(&con[i], &uip_conns[i])) {
            printf("connection[");
            printf("%d", i);
            printf("] changed.\r\n");
        }
    }
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
bool UIPDebug::uip_debug_printcon(struct uip_conn* lhs, struct uip_conn* rhs) {
    bool    changed = false;
    if (!uip_ipaddr_cmp(lhs->ripaddr, rhs->ripaddr)) {
        printf(" ripaddr: ");
        uip_debug_printbytes((const uint8_t*)lhs->ripaddr, 4);
        printf(" -> ");
        uip_debug_printbytes((const uint8_t*)rhs->ripaddr, 4);
        printf("\r\n");
        uip_ipaddr_copy(lhs->ripaddr, rhs->ripaddr);
        changed = true;
    }

    if (lhs->lport != rhs->lport) {
        printf(" lport: ");
        printf("%d", htons(lhs->lport));
        printf(" -> ");
        printf("%d\r\n", htons(rhs->lport));
        lhs->lport = rhs->lport;
        changed = true;
    }

    if (lhs->rport != rhs->rport) {
        printf(" rport: ");
        printf("%d", htons(lhs->rport));
        printf(" -> ");
        printf("%d\r\n", htons(rhs->rport));
        lhs->rport = rhs->rport;
        changed = true;
    }

    if ((uint32_t) lhs->rcv_nxt[0] != (uint32_t) rhs->rcv_nxt[0]) {
        printf(" rcv_nxt: ");
        uip_debug_printbytes(lhs->rcv_nxt, 4);
        printf(" -> ");
        uip_debug_printbytes(rhs->rcv_nxt, 4);
        *((uint32_t*) &lhs->rcv_nxt[0]) = (uint32_t) rhs->rcv_nxt[0];
        printf("\r\n");
        changed = true;
    }

    if ((uint32_t) lhs->snd_nxt[0] != (uint32_t) rhs->snd_nxt[0]) {
        printf(" snd_nxt: ");
        uip_debug_printbytes(lhs->snd_nxt, 4);
        printf(" -> ");
        uip_debug_printbytes(rhs->snd_nxt, 4);
        *((uint32_t*) &lhs->snd_nxt[0]) = (uint32_t) rhs->snd_nxt[0];
        printf("\r\n");
        changed = true;
    }

    if (lhs->len != rhs->len) {
        printf(" len: ");
        printf("%d", lhs->len);
        printf(" -> ");
        printf("%d\r\n", rhs->len);
        lhs->len = rhs->len;
        changed = true;
    }

    if (lhs->mss != rhs->mss) {
        printf(" mss: ");
        printf("%d", lhs->mss);
        printf(" -> ");
        printf("%d\r\n", rhs->mss);
        lhs->mss = rhs->mss;
        changed = true;
    }

    if (lhs->initialmss != rhs->initialmss) {
        printf(" initialmss: ");
        printf("%d", lhs->initialmss);
        printf(" -> ");
        printf("%d\r\n", rhs->initialmss);
        lhs->initialmss = rhs->initialmss;
        changed = true;
    }

    if (lhs->sa != rhs->sa) {
        printf(" sa: ");
        printf("%d", lhs->sa);
        printf(" -> ");
        printf("%d", rhs->sa);
        lhs->sa = rhs->sa;
        changed = true;
    }

    if (lhs->sv != rhs->sv) {
        printf(" sv: ");
        printf("%d", lhs->sv);
        printf(" -> ");
        printf("%d\r\n", rhs->sv);
        lhs->sv = rhs->sv;
        changed = true;
    }

    if (lhs->rto != rhs->rto) {
        printf(" rto: ");
        printf("%d", lhs->rto);
        printf(" -> ");
        printf("%d\r\n", rhs->rto);
        lhs->rto = rhs->rto;
        changed = true;
    }

    if (lhs->tcpstateflags != rhs->tcpstateflags) {
        printf(" tcpstateflags: ");
        printf("%d", lhs->tcpstateflags);
        printf(" -> ");
        printf("%d\r\n", rhs->tcpstateflags);
        lhs->tcpstateflags = rhs->tcpstateflags;
        changed = true;
    }

    if (lhs->timer != rhs->timer) {
        printf(" timer: ");
        printf("%d", lhs->timer);
        printf(" -> ");
        printf("%d\r\n", rhs->timer);
        lhs->timer = rhs->timer;
        changed = true;
    }

    if (lhs->nrtx != rhs->nrtx) {
        printf(" nrtx: ");
        printf("%d", lhs->nrtx);
        printf(" -> ");
        printf("%d\r\n", rhs->nrtx);
        lhs->nrtx = rhs->nrtx;
        changed = true;
    }

    return changed;
}

/**
 * @brief
 * @note
 * @param
 * @retval
 */
void UIPDebug::uip_debug_printbytes(const uint8_t* data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        printf("%d", data[i]);
        if (i < len - 1)
            printf(",");
    }
}
#endif

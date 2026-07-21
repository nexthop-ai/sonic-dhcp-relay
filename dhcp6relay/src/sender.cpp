#include "sender.h"
#include <syslog.h>
#include <errno.h>
#include <cstring>
#include <arpa/inet.h>

// Caller passes errno explicitly so the reason reflects the failed send.
// dest_ip falls back to the target address when the context omits it.
static void log_dhcp_relay_send_failure(const dhcp_relay_send_context *ctx,
                                        const struct sockaddr_in6 *target, int err)
{
    char target_ip[INET6_ADDRSTRLEN] = "unknown";

    if (target != nullptr) {
        inet_ntop(AF_INET6, &target->sin6_addr, target_ip, INET6_ADDRSTRLEN);
    }

    const char *protocol = (ctx && ctx->protocol) ? ctx->protocol : "DHCPv6";
    const char *downlink = (ctx && ctx->downlink) ? ctx->downlink : "unknown";
    const char *uplink = (ctx && ctx->uplink) ? ctx->uplink : "unknown";
    const char *dest_ip = (ctx && ctx->dest_ip) ? ctx->dest_ip : target_ip;
    const char *vrf = (ctx && ctx->vrf) ? ctx->vrf : "default";

    syslog(LOG_ERR,
           "DHCP_RELAY_SEND_FAIL protocol=%s downlink=%s uplink=%s "
           "dest_ip=%s vrf=%s reason=\"%s\"",
           protocol, downlink, uplink, dest_ip, vrf, strerror(err));
}

/**
 * @code                            bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in6 target, uint32_t n, const dhcp_relay_send_context *ctx);
 *
 * @brief                           send udp packet and return true if successful
 *
 * @param *buffer                   message buffer
 * @param sockaddr_in6 target       target socket
 * @param n                         length of message
 * @param ctx                       optional relay context for structured send-failure logs
 *
 * @return boolean   True if packet successfully sent
 */
bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in6 target, uint32_t n,
              const dhcp_relay_send_context *ctx) {
    if(sendto(sock, buffer, n, 0, (const struct sockaddr *)&target, sizeof(target)) == -1) {
        log_dhcp_relay_send_failure(ctx, &target, errno);
        return false;
    }
    return true;
}

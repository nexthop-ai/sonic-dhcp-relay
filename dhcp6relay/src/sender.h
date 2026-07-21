#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

// Keep in sync with dhcp4relay/src/dhcp4_sender.h.
struct dhcp_relay_send_context {
    const char *protocol;
    const char *downlink;
    const char *uplink;
    const char *dest_ip;
    const char *vrf;
};

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
              const dhcp_relay_send_context *ctx = nullptr);

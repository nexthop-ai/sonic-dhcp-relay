#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

#define BOOTP_MIN_LEN 300

// Keep in sync with dhcp6relay/src/sender.h.
struct dhcp_relay_send_context {
    const char *protocol;
    const char *downlink;
    const char *uplink;
    const char *dest_ip;
    const char *vrf;
};

/**
 * @code                            bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in target, uint32_t len, in_addr src_ip, bool use_src_ip, bool pad, const dhcp_relay_send_context *ctx);
 *
 * @brief                           send udp packet and return true if successful
 *
 * @param *buffer                   message buffer
 * @param sockaddr_in  target       target socket
 * @param len                         length of message
 * @param src_ip                    source IP address as string (optional)
 * @param use_src_ip                if true, use src_ip as source address
 * @param pad                       if true, do padding
 * @param ctx                       optional relay context for structured send-failure logs
 *
 * @return boolean   True if packet successfully sent
 */
bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in target, uint32_t len, in_addr src_ip,
              bool use_src_ip, bool pad, const dhcp_relay_send_context *ctx = nullptr);

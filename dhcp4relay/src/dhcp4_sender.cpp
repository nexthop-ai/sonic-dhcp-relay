#include "dhcp4_sender.h"

#include <arpa/inet.h>
#include <errno.h>
#include "logger.h"

#include <cstring>

// Caller passes errno explicitly so the reason reflects the failed send.
// dest_ip falls back to the target address when the context omits it.
static void log_dhcp_relay_send_failure(const dhcp_relay_send_context *ctx,
                                        const struct sockaddr_in *target, int err)
{
    char target_ip[INET_ADDRSTRLEN] = "unknown";

    if (target != nullptr) {
        inet_ntop(AF_INET, &target->sin_addr, target_ip, INET_ADDRSTRLEN);
    }

    const char *protocol = (ctx != nullptr && ctx->protocol != nullptr) ? ctx->protocol : "DHCPv4";
    const char *downlink = (ctx != nullptr && ctx->downlink != nullptr) ? ctx->downlink : "unknown";
    const char *uplink = (ctx != nullptr && ctx->uplink != nullptr) ? ctx->uplink : "unknown";
    const char *dest_ip = (ctx != nullptr && ctx->dest_ip != nullptr) ? ctx->dest_ip : target_ip;
    const char *vrf = (ctx != nullptr && ctx->vrf != nullptr) ? ctx->vrf : "default";

    SWSS_LOG_ERROR("DHCP_RELAY_SEND_FAIL protocol=%s downlink=%s uplink=%s "
                   "dest_ip=%s vrf=%s reason=\"%s\"",
                   protocol, downlink, uplink, dest_ip, vrf, strerror(err));
}

/**
 * @code                            bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in target, uint32_t len, in_addr src_ip, bool use_src_ip, bool pad, const dhcp_relay_send_context *ctx);
 *
 * @brief                           send udp packet and return true if successful
 *
 * @param *buffer                   message buffer
 * @param sockaddr_in  target       target socket
 * @param len                       length of message
 * @param src_ip                    source IP address as string (optional)
 * @param use_src_ip                if true, use src_ip as source address
 * @param pad                       if true, do padding
 * @param ctx                       optional relay context for structured send-failure logs
 *
 * @return boolean   True if packet successfully sent
 */
#ifndef UNIT_TEST
bool send_udp(int sock, uint8_t *buffer, struct sockaddr_in target, uint32_t len, in_addr src_ip,
              bool use_src_ip, bool pad, const dhcp_relay_send_context *ctx) {
   /* Pad additional bytes if length is lesser than 300
    * to make DHCP packet length to minimum of 300 bytes */
   if (pad && len < BOOTP_MIN_LEN) {
      auto pad_len = BOOTP_MIN_LEN - len;
      memset(buffer+len, 0, pad_len);
      len = BOOTP_MIN_LEN;
   }

   if (use_src_ip && src_ip.s_addr != 0) {
        // Enable IP_PKTINFO on the socket
        int on = 1;
        setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on));

        struct msghdr msg = {};
        struct iovec iov = {};
        char cmsgbuf[CMSG_SPACE(sizeof(struct in_pktinfo))];

        iov.iov_base = buffer;
        iov.iov_len = len;

        msg.msg_name = &target;
        msg.msg_namelen = sizeof(target);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgbuf;
        msg.msg_controllen = sizeof(cmsgbuf);

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

        struct in_pktinfo *pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);
        memset(pktinfo, 0, sizeof(struct in_pktinfo));
        pktinfo->ipi_spec_dst = src_ip;

        msg.msg_controllen = cmsg->cmsg_len;

        if (sendmsg(sock, &msg, 0) == -1) {
            log_dhcp_relay_send_failure(ctx, &target, errno);
            return false;
        }
    } else {
        if (sendto(sock, buffer, len, 0, (const struct sockaddr *)&target, sizeof(target)) == -1) {
            log_dhcp_relay_send_failure(ctx, &target, errno);
            return false;
        }
    }
    return true;
}
#endif

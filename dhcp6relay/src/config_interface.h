#pragma once

#include <string>
#include <unordered_map>
#include <boost/thread.hpp>
#include "subscriberstatetable.h"
#include "select.h"
#include "relay.h"

extern bool dual_tor_sock;

struct swssNotification {
    std::unordered_map<std::string, relay_config> vlans;
    swss::SubscriberStateTable *ipHelpersTable;
};

/**
 * @code                void initialize_swss()
 * 
 * @brief               initialize DB tables and start SWSS listening thread
 *
 * @return              none
 */
void initialize_swss(std::unordered_map<std::string, relay_config> &vlans);

/**
 * @code                void deinitialize_swss()
 * 
 * @brief               deinitialize DB interface and join SWSS listening thread
 *
 * @return              none
 */
void deinitialize_swss();

/**
 * @code                void get_dhcp(std::unordered_map<std::string, relay_config> &vlans, swss::SubscriberStateTable *ipHelpersTable, bool dynamic,
 *                                    std::shared_ptr<swss::DBConnector> config_db)
 * 
 * @brief               initialize and get vlan information from DHCP_RELAY
 *
 * @return              none
 */
void get_dhcp(std::unordered_map<std::string, relay_config> &vlans, swss::SubscriberStateTable *ipHelpersTable, bool dynamic,
              std::shared_ptr<swss::DBConnector> config_db);

/**
 * @code                    void handleRelayNotification(swss::SubscriberStateTable &ipHelpersTable, std::unordered_map<std::string, relay_config> &vlans,
 *                                                       std::shared_ptr<swss::DBConnector> config_db)
 * 
 * @brief                   handles DHCPv6 relay configuration change notification
 *
 * @param ipHelpersTable    DHCP table
 * @param vlans             map of vlans/argument config that contains strings of server and option
 *
 * @return                  none
 */
void handleRelayNotification(swss::SubscriberStateTable &ipHelpersTable, std::unordered_map<std::string, relay_config> &vlans,
                             std::shared_ptr<swss::DBConnector> config_db);

/**
 * @code                    void processRelayNotification(std::deque<swss::KeyOpFieldsValuesTuple> &entries, std::unordered_map<std::string, relay_config> &vlans,
 *                                                        std::shared_ptr<swss::DBConnector> config_db)
 * 
 * @brief                   process DHCPv6 relay servers and options configuration change notification
 *
 * @param entries           queue of std::tuple<std::string, std::string, std::vector<FieldValueTuple>> entries in DHCP table
 * @param context           map of vlans/argument config that contains strings of server and option
 *
 * @return                  none
 */
void processRelayNotification(std::deque<swss::KeyOpFieldsValuesTuple> &entries, std::unordered_map<std::string, relay_config> &vlans,
                              std::shared_ptr<swss::DBConnector> config_db);

/**
<<<<<<< HEAD
 * @code                    bool check_is_lla_ready(std::string vlan)
=======
 * @code                    void processVlanMemberNotification(std::deque<swss::KeyOpFieldsValuesTuple> &entries,
 *                                                             std::unordered_map<std::string, relay_config> &interfaces)
 *
 * @brief                   process VLAN_MEMBER add/del notifications so that DHCPv6 relay picks up
 *                          member changes on the fly (no dhcp_relay restart required). Updates the
 *                          global member-port -> VLAN interface_map for VLANs that have relay config
 *                          and whose link-local address is already up.
 *
 * @param entries           queue of VLAN_MEMBER table change entries (key form "<Vlan>|<member>")
 * @param interfaces        map of relay interface configs keyed by VLAN name
 *
 * @return                  none
 */
void processVlanMemberNotification(std::deque<swss::KeyOpFieldsValuesTuple> &entries,
                                   std::unordered_map<std::string, relay_config> &interfaces);

/**
 * @code                    bool check_is_lla_ready(std::string interface)
>>>>>>> 74c0e86 (NOS-7408: dhcp6relay applies VLAN member add/del dynamically (#43))
 * 
 * @brief                   Check whether link local address appear in vlan interface
 *
 * @param vlan              string of vlan name
 *
 * @return                  bool value indicates whether lla ready
 */
bool check_is_lla_ready(std::string vlan);

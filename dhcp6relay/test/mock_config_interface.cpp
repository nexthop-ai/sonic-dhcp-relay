#include "mock_config_interface.h"

using namespace ::testing;

TEST(configInterface, initialize_swss) {
  std::shared_ptr<swss::DBConnector> config_db = std::make_shared<swss::DBConnector> ("CONFIG_DB", 0);
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_servers@", "fc02:2000::1,fc02:2000::2,fc02:2000::3,fc02:2000::4");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|rfc6939_support", "false");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|interface_id", "true");
  config_db->hset("VLAN_INTERFACE|Vlan1000|fc02:1000::1", "", "");
  std::unordered_map<std::string, relay_config> vlans;
  ASSERT_NO_THROW(initialize_swss(vlans));
  EXPECT_EQ(vlans.size(), 1);
}

TEST(configInterface, deinitialize_swss) {
  ASSERT_NO_THROW(deinitialize_swss());
}

TEST(configInterface, get_dhcp) {
  std::shared_ptr<swss::DBConnector> config_db = std::make_shared<swss::DBConnector> ("CONFIG_DB", 0);
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_servers@", "fc02:2000::1,fc02:2000::2,fc02:2000::3,fc02:2000::4");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|rfc6939_support", "false");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|interface_id", "true");
  swss::SubscriberStateTable ipHelpersTable(config_db.get(), "DHCP_RELAY");
  std::unordered_map<std::string, relay_config> vlans;

  ASSERT_NO_THROW(get_dhcp(vlans, &ipHelpersTable, false, config_db));
  EXPECT_EQ(vlans.size(), 0);

  swssSelect.addSelectable(&ipHelpersTable);

  ASSERT_NO_THROW(get_dhcp(vlans, &ipHelpersTable, false, config_db));
  EXPECT_EQ(vlans.size(), 1);
}

TEST(configInterface, handleRelayNotification) {
  std::shared_ptr<swss::DBConnector> cfg_db = std::make_shared<swss::DBConnector> ("CONFIG_DB", 0);
  swss::SubscriberStateTable ipHelpersTable(cfg_db.get(), "DHCP_RELAY");
  std::unordered_map<std::string, relay_config> vlans;
  handleRelayNotification(ipHelpersTable, vlans, cfg_db);
}

TEST(configInterface, processRelayNotification) {  
  std::shared_ptr<swss::DBConnector> config_db = std::make_shared<swss::DBConnector> ("CONFIG_DB", 0);
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_servers@", "fc02:2000::1,fc02:2000::2,fc02:2000::3,fc02:2000::4");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|rfc6939_support", "false");
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_option|interface_id", "true");
  swss::SubscriberStateTable ipHelpersTable(config_db.get(), "DHCP_RELAY");
  swssSelect.addSelectable(&ipHelpersTable);
  std::deque<swss::KeyOpFieldsValuesTuple> entries;
  ipHelpersTable.pops(entries);
  std::unordered_map<std::string, relay_config> vlans;

  processRelayNotification(entries, vlans, config_db);

  EXPECT_EQ(vlans.size(), 1);
  EXPECT_FALSE(vlans["Vlan1000"].is_option_79);
  EXPECT_TRUE(vlans["Vlan1000"].is_interface_id);
  EXPECT_FALSE(vlans["Vlan1000"].state_db);
}

TEST(configInterface, processVlanMemberNotificationFromConfigDb) {
  std::shared_ptr<swss::DBConnector> config_db = std::make_shared<swss::DBConnector> ("CONFIG_DB", 0);
  // Clear stale db
  config_db->flushdb();
  interface_map.clear();

  // Vlan1000: relay configured, LLA marked ready below. Vlan2000: relay configured, LLA not ready.
  config_db->hset("DHCP_RELAY|Vlan1000", "dhcpv6_servers", "fc02:2000::1");
  config_db->hset("VLAN_INTERFACE|Vlan1000|fc02:1000::1", "", "");
  config_db->hset("DHCP_RELAY|Vlan2000", "dhcpv6_servers", "fc02:2000::1");
  config_db->hset("VLAN_INTERFACE|Vlan2000|fc02:1001::1", "", "");

  swss::SubscriberStateTable relayTable(config_db.get(), "DHCP_RELAY");
  swssSelect.addSelectable(&relayTable);
  std::deque<swss::KeyOpFieldsValuesTuple> relay_entries;
  relayTable.pops(relay_entries);

  std::unordered_map<std::string, relay_config> interfaces;
  processRelayNotification(relay_entries, interfaces, config_db);
  EXPECT_EQ(interfaces.count("Vlan1000"), 1);
  EXPECT_EQ(interfaces.count("Vlan2000"), 1);

  interfaces["Vlan1000"].is_lla_ready = true;

  // Add VLAN members in CONFIG_DB after the relay config is built, then source the notifications.
  config_db->hset("VLAN_MEMBER|Vlan1000|Ethernet8", "tagging_mode", "untagged");
  config_db->hset("VLAN_MEMBER|Vlan2000|Ethernet10", "tagging_mode", "untagged");
  config_db->hset("VLAN_MEMBER|Vlan4000|Ethernet9", "tagging_mode", "untagged");

  swss::SubscriberStateTable vlanMemberTable(config_db.get(), "VLAN_MEMBER");
  swssSelect.addSelectable(&vlanMemberTable);
  std::deque<swss::KeyOpFieldsValuesTuple> member_entries;
  vlanMemberTable.pops(member_entries);

  interface_map.clear();
  processVlanMemberNotification(member_entries, interfaces);

  EXPECT_EQ(interface_map["Ethernet8"], "Vlan1000");
  EXPECT_EQ(interface_map.find("Ethernet10"), interface_map.end());
  EXPECT_EQ(interface_map.find("Ethernet9"), interface_map.end());

  //Clean up
  interface_map.clear();
  config_db->flushdb();
}

MOCK_GLOBAL_FUNC0(stopSwssNotificationPoll, void(void));

TEST(configInterface, stopSwssNotificationPoll) {
  EXPECT_GLOBAL_CALL(stopSwssNotificationPoll, stopSwssNotificationPoll()).Times(1);
  ASSERT_NO_THROW(stopSwssNotificationPoll());
}

TEST(configInterface, check_is_lla_ready) {
  EXPECT_FALSE(check_is_lla_ready("Vlan1000"));
}

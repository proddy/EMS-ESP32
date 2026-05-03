/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2025  emsesp.org
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "network.h"

#include "emsesp.h"

namespace emsesp {

uuid::log::Logger Network::logger_{F_(network), uuid::log::Facility::KERN};

void Network::begin() {
#ifndef EMSESP_STANDALONE
    // pull all settings and store locally
    EMSESP::esp32React.getNetworkSettingsService()->read([&](NetworkSettings & settings) {
        enableMDNS_     = settings.enableMDNS;
        staticIPConfig_ = settings.staticIPConfig;
        localIP_        = settings.localIP;
        gatewayIP_      = settings.gatewayIP;
        subnetMask_     = settings.subnetMask;
        dnsIP1_         = settings.dnsIP1;
        dnsIP2_         = settings.dnsIP2;
        hostname_       = settings.hostname;
        ssid_           = settings.ssid;
        password_       = settings.password;
        bandwidth20_    = settings.bandwidth20;
        nosleep_        = settings.nosleep;
        tx_power_       = settings.tx_power;
        bssid_          = settings.bssid;
    });

    // read Ethernet settings
    EMSESP::webSettingsService.read([&](WebSettings & settings) {
        phy_type_       = settings.phy_type;
        eth_power_      = settings.eth_power;
        eth_phy_addr_   = settings.eth_phy_addr;
        eth_clock_mode_ = settings.eth_clock_mode;
    });

    // get Access Point settings
    EMSESP::esp32React.getAPSettingsService()->read([&](APSettings & settings) {
        ap_provisionMode_ = settings.provisionMode;
        ap_ssid_          = settings.ssid;
        ap_password_      = settings.password;
        ap_channel_       = settings.channel;
        ap_ssid_hidden_   = settings.ssidHidden;
        ap_max_clients_   = settings.maxClients;
        ap_localIP_       = settings.localIP;
        ap_gatewayIP_     = settings.gatewayIP;
        ap_subnetMask_    = settings.subnetMask;
    });

    // Initialise WiFi - we only do this once, when the network service is started.

    // We want the device to come up in opmode=0 (WIFI_OFF), which is not the default after a flash erase.
    // Persistence is true by default, so this WiFi.mode() call writes opmode=0 to NVS for future boots.
    // WiFi.mode(WIFI_OFF);

    // if Wifi is disabled, with no SSID, stop here
    if (ssid_.isEmpty()) {
        WiFi.mode(WIFI_OFF);
        return;
    }

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);                   // wipe old settings in NVS
    WiFi.setHostname(hostname_.c_str());     // updates shared default_hostname buffer
    WiFi.enableSTA(true);                    // creates the STA netif
    WiFi.STA.setHostname(hostname_.c_str()); // pushes to esp_netif_set_hostname
    WiFi.enableIPv6(true);
    if (staticIPConfig_) {
        WiFi.config(localIP_, gatewayIP_, subnetMask_, dnsIP1_, dnsIP2_); // configure for static IP
    }

    // www.esp32.com/viewtopic.php?t=12055
    if (bandwidth20_) {
        esp_wifi_set_bandwidth(static_cast<wifi_interface_t>(ESP_IF_WIFI_STA), WIFI_BW_HT20);
    } else {
        esp_wifi_set_bandwidth(static_cast<wifi_interface_t>(ESP_IF_WIFI_STA), WIFI_BW_HT40);
    }
    if (nosleep_) {
        WiFi.setSleep(false); // turn off sleep - WIFI_PS_NONE
    }

    wifi_connect_pending_ = false; // set before begin() so the event handlers can race-clear it safely

    // scan settings give connect issues since arduino 2.0.14 and arduino 3.x.x with some wifi systems
    // WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN); // default is FAST_SCAN
    // WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL); // is default, no need to set

    // arduino-esp32's WiFi.onEvent() simply appends to an internal callback list with no
    // de-duplication, so register the lambdas only once across the lifetime of this Network instance
    if (!wifi_events_registered_) {
        wifi_events_registered_ = true;

        // Defer Tx power setting until STA is actually started. Calling WiFi.setTxPower() before
        // WIFI_EVENT_STA_START fires would fail with "Neither AP or STA has been started" because
        // WiFi.STA.started() only flips after esp_wifi_start() raises the event asynchronously.
        WiFi.onEvent(
            [this](WiFiEvent_t /*event*/, WiFiEventInfo_t /*info*/) {
#ifdef BOARD_C3_MINI_V1
                // always hardcode Tx power for Wemos C3 Mini v1
                // v1 needs this value, see https://github.com/emsesp/EMS-ESP32/pull/620#discussion_r993173979
                // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
                WiFi.setTxPower(WIFI_POWER_8_5dBm);
#else
                setWiFiPower(tx_power_);
#endif
            },
            ARDUINO_EVENT_WIFI_STA_START);

        // capture the WIFI_REASON_* code on every STA disconnect event so check_connection() can
        // log a meaningful reason when its periodic poll notices we're no longer associated.
        // Also release the connect-pending guard so the next loop tick can issue a fresh WiFi.begin().
        // The first STA_DISCONNECTED after boot is suppressed because arduino-esp32 hard-codes a
        // "retry once on first_connect" inside its own STA event handler (see STA.cpp), so a
        // transient initial AUTH_FAIL / NO_AP_FOUND / etc. is automatically retried and almost
        // always succeeds; logging it as a WARNING is misleading noise.
        WiFi.onEvent(
            [this](WiFiEvent_t /*event*/, WiFiEventInfo_t info) {
                last_disconnect_reason_ = info.wifi_sta_disconnected.reason;
                wifi_connect_pending_   = false;
                if (wifi_ever_connected_) {
                    LOG_WARNING("WiFi disconnected (reason: %s)", disconnectReason(last_disconnect_reason_));
                } else {
                    LOG_WARNING("WiFi initial connect attempt failed (reason: %s)", disconnectReason(last_disconnect_reason_));
                }
            },
            ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

        // clear the saved reason and the connect-pending guard on a fresh STA association,
        // and latch wifi_ever_connected_ so future disconnects log as warnings
        WiFi.onEvent(
            [this](WiFiEvent_t /*event*/, WiFiEventInfo_t /*info*/) {
                last_disconnect_reason_ = 0;
                wifi_connect_pending_   = false;
                wifi_ever_connected_    = true;
            },
            ARDUINO_EVENT_WIFI_STA_GOT_IP);
    }

#endif
}

// format the BSSID (MAC address) of the network interface
bool Network::formatBSSID(const String & bssid, uint8_t (&mac)[6]) {
#ifndef EMSESP_STANDALONE
    uint tmp[6];
    if (bssid.isEmpty() || sscanf(bssid.c_str(), "%X:%X:%X:%X:%X:%X", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]) != 6) {
        return false;
    }
    for (uint8_t i = 0; i < 6; i++) {
        mac[i] = static_cast<uint8_t>(tmp[i]);
    }
#endif
    return true;
}

// get the local IP address of the network interface
std::string Network::getLocalIP() const {
    switch (network_iface_) {
#ifndef EMSESP_STANDALONE
    case NetIface::AP:
        return WiFi.softAPIP().toString().c_str();
    case NetIface::WIFI:
        return WiFi.localIP().toString().c_str();
    case NetIface::ETHERNET:
        return ETH.localIP().toString().c_str();
    case NetIface::NONE:
#endif
    default:
        return "";
    }
}

// get the MAC address of the network interface
std::string Network::getMacAddress() const {
    switch (network_iface_) {
#ifndef EMSESP_STANDALONE
    case NetIface::AP:
        return WiFi.softAPmacAddress().c_str();
    case NetIface::WIFI:
        return WiFi.macAddress().c_str();
    case NetIface::ETHERNET:
        return ETH.macAddress().c_str();
    case NetIface::NONE:
#endif
    default:
        return "";
    }
}

// get the number of stations connected to the AP
uint8_t Network::getStationNum() const {
#ifndef EMSESP_STANDALONE
    return network_iface_ == NetIface::AP ? WiFi.softAPgetStationNum() : 0;
#else
    return 0;
#endif
}

// disconnect all WiFi, Eth and AP
// so we can starts searching again to reconnect
void Network::reconnect() {
    LOG_DEBUG("Reconnecting all networks");

#ifndef EMSESP_STANDALONE
    // disconnect WiFi
    if (wifi_connected()) {
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_STA); // reset mode
    }

    // disconnect AP
    if (WiFi.getMode() & WIFI_AP) {
        stopAP();
    }
#endif

    // reset network state
    network_ip_             = 0;
    network_iface_          = NetIface::NONE;
    has_ipv6_               = false;
    juststopped_            = false;
    wifi_connect_pending_   = false;
    last_disconnect_reason_ = 0;
    connect_retry_          = 0;
    reconnect_count_        = 0;

    // reload the network settings and apply them
    begin();
}

// network loop, looking for new and disconnecting networks
void Network::loop() {
#ifndef EMSESP_STANDALONE
    // if we already have a Wifi or Ethernet connection then re-check every NETWORK_RECONNECTION_DELAY_LONG, otherwise NETWORK_RECONNECTION_DELAY_SHORT
    const unsigned long currentMillis = millis();
    const uint32_t      reconnectDelay =
        (network_iface_ == NetIface::WIFI || network_iface_ == NetIface::ETHERNET) ? NETWORK_RECONNECTION_DELAY_LONG : NETWORK_RECONNECTION_DELAY_SHORT;
    if (!lastConnectionAttempt_ || static_cast<uint32_t>(currentMillis - lastConnectionAttempt_) >= reconnectDelay) {
        lastConnectionAttempt_ = currentMillis;

        // manage network interfaces
        startEthernet(); // Ethernet
        startWIFI();     // WiFi
        startAP();       // Captive Portal (AP)

        // already have a connection: verify it's still alive
        // or trigger if the WiFi handshaked failed on the WiFi.begin() call
        if (network_ip_ != 0 || last_disconnect_reason_ != 0) {
            checkConnection();
        }

        findNetworks(); // detect any new network connections
    }

    // process DNS requests for the captive portal while the soft-AP is up
    if (ap_dnsServer_) {
        ap_dnsServer_->processNextRequest();
    }
#endif
}

// Re-validate the currently active connection
// if a netif is no longer up or has lost its IP (cable unplugged, AP gone, DHCP lease lost, ...) we drop our state so
// find_networks() can pick up a new one
void Network::checkConnection() {
    if (network_iface_ == NetIface::NONE) {
        return;
    }

#ifndef EMSESP_STANDALONE
    bool still_up = false;
    for (esp_netif_t * netif = esp_netif_next_unsafe(NULL); netif != NULL; netif = esp_netif_next_unsafe(netif)) {
        if (iface_from_desc(esp_netif_get_desc(netif)) != network_iface_) {
            continue;
        }
        esp_netif_ip_info_t ip_info = {};
        if (esp_netif_is_netif_up(netif) && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK && ip_info.ip.addr == network_ip_) {
            still_up = true;
        }
        break; // only one active netif per kind in our world (sta / eth / ap)
    }

    if (!still_up) {
        if (network_iface_ == NetIface::WIFI) {
            uint8_t reason = last_disconnect_reason_;
            if (reason == 0) {
                reason = WIFI_REASON_UNSPECIFIED; // event hasn't fired yet (or was cleared); avoid logging "0"
            }
            wifi_connect_pending_ = false;
            LOG_INFO("WiFi connection lost (reason %u: %s)", reason, disconnectReason(reason));
        } else if (network_iface_ == NetIface::ETHERNET) {
            LOG_INFO("Ethernet connection lost");
        }
        juststopped_   = true;
        network_iface_ = NetIface::NONE;
        network_ip_    = 0;
        has_ipv6_      = false;
    }
#endif
}

// set the WiFi TxPower based on the RSSI (signal strength), picking the lowest value
// code is based of RSSI (signal strength) and copied from Tasmota's WiFiSetTXpowerBasedOnRssi() which is copied ESPEasy's ESPEasyWifi.SetWiFiTXpower() function
// Range ESP32  : 2dBm - 20dBm
// 802.11b - wifi1
// 802.11a - wifi2
// 802.11g - wifi3
// 802.11n - wifi4
// 802.11ac - wifi5
// 802.11ax - wifi6
// tx_power is the Tx power to set, 0 for auto
void Network::setWiFiPower(uint8_t tx_power) {
#ifndef EMSESP_STANDALONE
    if (tx_power != 0) {
        if (!WiFi.setTxPower(static_cast<wifi_power_t>(tx_power))) {
            emsesp::EMSESP::logger().warning("Failed to set WiFi Tx Power");
        }
        return;
    }

    int max_tx_pwr = MAX_TX_PWR_DBM_n;         // assume wifi4
    int threshold  = WIFI_SENSITIVITY_n + 120; // Margin in dBm * 10 on top of threshold

    // Assume AP sends with max set by ETSI standard.
    // 2.4 GHz: 100 mWatt (20 dBm)
    // US and some other countries allow 1000 mW (30 dBm)
    int rssi    = WiFi.RSSI() * 10;
    int newrssi = rssi - 200; // We cannot send with over 20 dBm, thus it makes no sense to force higher TX power all the time.

    int min_tx_pwr = 0;
    if (newrssi < threshold) {
        min_tx_pwr = threshold - newrssi;
    }
    if (min_tx_pwr > max_tx_pwr) {
        min_tx_pwr = max_tx_pwr;
    }

    // from WiFIGeneric.h use:
    wifi_power_t p = WIFI_POWER_2dBm;
    if (min_tx_pwr > 185)
        p = WIFI_POWER_19_5dBm;
    else if (min_tx_pwr > 170)
        p = WIFI_POWER_18_5dBm;
    else if (min_tx_pwr > 150)
        p = WIFI_POWER_17dBm;
    else if (min_tx_pwr > 130)
        p = WIFI_POWER_15dBm;
    else if (min_tx_pwr > 110)
        p = WIFI_POWER_13dBm;
    else if (min_tx_pwr > 85)
        p = WIFI_POWER_11dBm;
    else if (min_tx_pwr > 70)
        p = WIFI_POWER_8_5dBm;
    else if (min_tx_pwr > 50)
        p = WIFI_POWER_7dBm;
    else if (min_tx_pwr > 20)
        p = WIFI_POWER_5dBm;

#if defined(EMSESP_DEBUG)
    // emsesp::EMSESP::logger().debug("Recommended WiFi Tx Power (set_power %d, new power %d, rssi %d, threshold %d)", min_tx_pwr / 10, p, rssi, threshold);
#endif

    if (!WiFi.setTxPower(p)) {
        emsesp::EMSESP::logger().warning("Failed to set WiFi Tx Power!!");
    }
#endif
}

// start the multicast UDP service so EMS-ESP is discoverable via .local
void Network::startmDNS() const {
#ifndef EMSESP_STANDALONE
    MDNS.end();

    if (!enableMDNS_) {
        return;
    }

    if (!MDNS.begin(emsesp::EMSESP::system_.hostname().c_str())) {
        emsesp::EMSESP::logger().warning("Failed to start mDNS Responder service");
        return;
    }

    std::string address_s = emsesp::EMSESP::system_.hostname() + ".local";

    MDNS.addService("http", "tcp", 80);   // add our web server and rest API
    MDNS.addService("telnet", "tcp", 23); // add our telnet console
    MDNS.addServiceTxt("http", "tcp", "address", address_s.c_str());

    emsesp::EMSESP::logger().info("Starting mDNS Responder service");
#endif
}

// WiFi disconnect reason code to string
const char * Network::disconnectReason(uint8_t code) {
#ifndef EMSESP_STANDALONE
    switch (code) {
    case WIFI_REASON_UNSPECIFIED: // = 1,
        return "unspecified";
    case WIFI_REASON_AUTH_EXPIRE: // = 2,
        return "auth expire";
    case WIFI_REASON_AUTH_LEAVE: // = 3,
        return "auth leave";
    case WIFI_REASON_ASSOC_EXPIRE: // = 4,
        return "assoc expired";
    case WIFI_REASON_ASSOC_TOOMANY: // = 5,
        return "assoc too many";
    case WIFI_REASON_NOT_AUTHED: // = 6,
        return "not authenticated";
    case WIFI_REASON_NOT_ASSOCED: // = 7,
        return "not assoc";
    case WIFI_REASON_ASSOC_LEAVE: // = 8,
        return "assoc leave";
    case WIFI_REASON_ASSOC_NOT_AUTHED: // = 9,
        return "assoc not authed";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD: // = 10,
        return "disassoc powerCAP bad";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD: // = 11,
        return "disassoc supchan bad";
    case WIFI_REASON_IE_INVALID: // = 13,
        return "IE invalid";
    case WIFI_REASON_MIC_FAILURE: // = 14,
        return "MIC failure";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: // = 15,
        return "4way handshake timeout";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: // = 16,
        return "group key-update timeout";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS: // = 17,
        return "IE in 4way differs";
    case WIFI_REASON_GROUP_CIPHER_INVALID: // = 18,
        return "group cipher invalid";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID: // = 19,
        return "pairwise cipher invalid";
    case WIFI_REASON_AKMP_INVALID: // = 20,
        return "AKMP invalid";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION: // = 21,
        return "unsupported RSN_IE version";
    case WIFI_REASON_INVALID_RSN_IE_CAP: // = 22,
        return "invalid RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED: // = 23,
        return "802 X1 auth failed";
    case WIFI_REASON_CIPHER_SUITE_REJECTED: // = 24,
        return "cipher suite rejected";
    case WIFI_REASON_BEACON_TIMEOUT: // = 200,
        return "beacon timeout";
    case WIFI_REASON_NO_AP_FOUND: // = 201,
        return "no AP found";
    case WIFI_REASON_AUTH_FAIL: // = 202,
        return "auth fail";
    case WIFI_REASON_ASSOC_FAIL: // = 203,
        return "assoc fail";
    case WIFI_REASON_HANDSHAKE_TIMEOUT: // = 204,
        return "handshake timeout";
    case WIFI_REASON_CONNECTION_FAIL: // 205,
        return "connection fail";
    case WIFI_REASON_AP_TSF_RESET: // 206,
        return "AP tsf reset";
    case WIFI_REASON_ROAMING: // 207,
        return "roaming";
    case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG: // 208,
        return "assoc comeback time too long";
    case WIFI_REASON_SA_QUERY_TIMEOUT: // 209,
        return "sa query timeout";
    default:
        return "unknown";
    }
#endif

    return "";
}

// WiFi management
void Network::startWIFI() {
#ifndef EMSESP_STANDALONE
    // exit if WiFi is already connected, or if we have no SSID or another Wifi.begin() is already in progress
    if (WiFi.isConnected() || ssid_.isEmpty() || wifi_connect_pending_) {
        return;
    }

    wifi_connect_pending_ = true;

    // LOG_DEBUG("WiFi connection with %s and %s", ssid_.c_str(), password_.c_str());

    // attempt to connect to the wifi network
    // the event handlers handle error handling and retries
    uint8_t bssid[6];
    if (formatBSSID(bssid_, bssid)) {
        WiFi.begin(ssid_.c_str(), password_.c_str(), 0, bssid);
    } else {
        WiFi.begin(ssid_.c_str(), password_.c_str());
    }
#endif
}

// Ethernet management
// Brings up the ETH driver / netif exactly once. After ETH.begin() returns true the driver
// continues to run autonomously (link negotiation, DHCP, etc); the loop must NOT call ETH.begin()
// again on every iteration because that thrashes the netif and can prevent DHCP from completing
void Network::startEthernet() {
#if CONFIG_IDF_TARGET_ESP32
    // already up and running, nothing to do
    if (eth_started_) {
        return;
    }

#ifndef EMSESP_STANDALONE

    // no ethernet present
    if (phy_type_ == PHY_type::PHY_TYPE_NONE) {
        return;
    }

    // reset power and add a delay as ETH doesn't not always start up correctly after a warm boot
    if (eth_power_ != -1) {
        pinMode(eth_power_, OUTPUT);
        digitalWrite(eth_power_, LOW);
        delay(500);
        digitalWrite(eth_power_, HIGH);
    }

    // call to ETH.begin(type, phy_addr, mdc, mdio, power, clock_mode)
    // mdc = 23 =  Pin# of the I²C clock signal for the Ethernet PHY - hardcoded
    // mdio = 18 = Pin# of the I²C IO signal for the Ethernet PHY - hardcoded
    // phy_addr = eth_phy_addr_ = I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
    // power = eth_power_ = Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
    // type = Type of the Ethernet PHY (LAN8720 or TLK110)
    // clock_mode =
    //  ETH_CLOCK_GPIO0_IN   = 0  RMII clock input to GPIO0
    //  ETH_CLOCK_GPIO0_OUT  = 1  RMII clock output from GPIO0
    //  ETH_CLOCK_GPIO16_OUT = 2  RMII clock output from GPIO16
    //  ETH_CLOCK_GPIO17_OUT = 3  RMII clock output from GPIO17, for 50hz inverted clock
    eth_phy_type_t type = (phy_type_ == PHY_type::PHY_TYPE_LAN8720)  ? ETH_PHY_LAN8720
                          : (phy_type_ == PHY_type::PHY_TYPE_TLK110) ? ETH_PHY_TLK110
                                                                     : ETH_PHY_RTL8201; // Type of the Ethernet PHY (LAN8720 or TLK110)
    if (ETH.begin(type, eth_phy_addr_, 23, 18, eth_power_, (eth_clock_mode_t)eth_clock_mode_)) {
        LOG_DEBUG("Ethernet started");
        eth_started_ = true;                // mark up; do not re-enter this block until reboot / explicit teardown
        ETH.setHostname(hostname_.c_str()); // Push hostname to the ETH netif immediately after it's created
        ETH.enableIPv6(true);
        if (staticIPConfig_) {
            ETH.config(localIP_, gatewayIP_, subnetMask_, dnsIP1_, dnsIP2_);
        }
    } else {
        LOG_ERROR("Failed to start Ethernet");
    }
#endif
#endif
}

// check if the network is connected and set network_ip_ / network_iface_ / has_ipv6_
// Iterates over every esp-netif that exists, prioritizing Ethernet > WiFi > AP
void Network::findNetworks() {
#ifndef EMSESP_STANDALONE

    // exit if already have a connection, unless in AP mode
    // when in AP mode, it will always try and connect to the WiFi
    // TODO what about if Eth drops and then comes back - we want to auto-switch?
    // if (network_ip_ != 0 && !(WiFi.getMode() & WIFI_AP)) {
    //     // for debugging only
    //     // const esp_ip4_addr_t ip4 = {.addr = network_ip_};
    //     // LOG_DEBUG("Network already connected via IPv4: " IPSTR, IP2STR(&ip4));
    //     return;
    // }

    struct NetInfo {
        esp_ip4_addr_t ip;
        esp_ip6_addr_t ip6;
        char           desc[8];
        bool           has_ipv6;
    } info = {};

    // Preference order: ETHERNET > WIFI (STA) > AP
    auto iface_priority = [](NetIface iface) -> uint8_t {
        switch (iface) {
        case NetIface::ETHERNET:
            return 3;
        case NetIface::WIFI:
            return 2;
        case NetIface::AP:
            return 1;
        case NetIface::NONE:
        default:
            return 0;
        }
    };

    NetIface best_iface = NetIface::NONE;
    for (esp_netif_t * netif = esp_netif_next_unsafe(NULL); netif != NULL; netif = esp_netif_next_unsafe(netif)) {
        const char *        desc    = esp_netif_get_desc(netif);
        bool                is_up   = esp_netif_is_netif_up(netif);
        esp_netif_ip_info_t ip_info = {};
        esp_err_t           err     = esp_netif_get_ip_info(netif, &ip_info);

        if (!is_up || err != ESP_OK || ip_info.ip.addr == 0) {
            continue;
        }

        const NetIface candidate = iface_from_desc(desc);
        if (iface_priority(candidate) <= iface_priority(best_iface)) {
            continue; // already have something at least as good
        }

        info.ip = ip_info.ip;
        if (desc) {
            strlcpy(info.desc, desc, sizeof(info.desc));
        }
        info.has_ipv6 = (esp_netif_get_ip6_linklocal(netif, &info.ip6) == ESP_OK);

        best_iface = candidate;
        if (best_iface == NetIface::ETHERNET) {
            break; // top priority, can't be beaten by anything later in the list
        }
    }

    // LOG_DEBUG("best_iface: %d, network_iface_: %d", best_iface, network_iface_);

    // if we have a connection and it's a new one, set it up
    if (best_iface != NetIface::NONE && best_iface != network_iface_) {
        network_ip_    = info.ip.addr;
        network_iface_ = iface_from_desc(info.desc); // "sta"/"ap"/"eth*"
        has_ipv6_      = info.has_ipv6;
        connect_retry_ = 0;

        LOG_INFO("Network connected via %s (IP: " IPSTR ")",
                 network_iface_ == NetIface::ETHERNET ? "Ethernet"
                 : network_iface_ == NetIface::WIFI   ? "WiFi"
                 : network_iface_ == NetIface::AP     ? "AP"
                                                      : "unknown",
                 IP2STR(&info.ip));

        // if we have a Eth or Wifi connection and the AP is running, stop it
        if (network_iface_ != NetIface::AP && WiFi.getMode() & WIFI_AP) {
            stopAP();
        }

        // count the number of restarts (for Wifi and Eth)
        if (juststopped_) {
            juststopped_ = false;
            reconnect_count_++;
        }

        // start mDNS for any real network interface (skip the SoftAP since the captive portal handles its own DNS)
        if (enableMDNS_ && network_iface_ != NetIface::AP && network_iface_ != NetIface::NONE) {
            startmDNS();
        }

        // fetch the versions.json file from emsesp.org
        EMSESP::webStatusService.schedule_versions_refresh();

        return;
    }

    // fallback, reset network state if nothing found
    if (best_iface == NetIface::NONE) {
        network_ip_    = 0;
        network_iface_ = NetIface::NONE;
        has_ipv6_      = false;
        connect_retry_++;
        LOG_DEBUG("No active network interfaces found yet (retry #%d)", connect_retry_);
    }

#endif
    return;
}

// access point (soft-AP) and the captive portal
void Network::startAP() {
#ifndef EMSESP_STANDALONE
    // Only start AP as a fallback if the Network has failed
    if (connect_retry_ < MAX_NETWORK_RECONNECTION_ATTEMPTS) {
        return;
    }

    // don't start the soft-AP if it is disabled, or Ethernet has taken over or we have a real WiFi connection or it's already running
    if (ap_provisionMode_ == AP_MODE_NEVER || network_connected() || WiFi.getMode() & WIFI_AP) {
        return;
    }

    WiFi.softAPenableIPv6(); // force IPv6, same as for STA - fixes https://github.com/emsesp/EMS-ESP32/issues/1922
    WiFi.softAPConfig(ap_localIP_, ap_gatewayIP_, ap_subnetMask_);
    esp_wifi_set_bandwidth(static_cast<wifi_interface_t>(ESP_IF_WIFI_AP), WIFI_BW_HT20);
    WiFi.softAP(ap_ssid_.c_str(), ap_password_.c_str(), ap_channel_, ap_ssid_hidden_, ap_max_clients_);
#if CONFIG_IDF_TARGET_ESP32C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html#about-wifi
#endif
    const IPAddress apIp = WiFi.softAPIP();
    LOG_INFO("Starting Access Point with captive portal on %u.%u.%u.%u", apIp[0], apIp[1], apIp[2], apIp[3]);

    // start DNS server for Captive Portal
    ap_dnsServer_ = new DNSServer;
    ap_dnsServer_->start(DNS_PORT, "*", apIp);
#endif
}

// stop AP
void Network::stopAP() {
    LOG_INFO("Stopping Access Point");
#ifndef EMSESP_STANDALONE
    WiFi.softAPdisconnect(true);
    if (ap_dnsServer_) {
        ap_dnsServer_->stop();
        delete ap_dnsServer_;
        ap_dnsServer_ = nullptr;
    }
#endif
}

} // namespace emsesp

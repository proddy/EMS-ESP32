#include "APSettingsService.h"

#include <emsesp.h>

APSettingsService::APSettingsService(AsyncWebServer * server, FS * fs, SecurityManager * securityManager)
    : _httpEndpoint(APSettings::read, APSettings::update, this, server, AP_SETTINGS_SERVICE_PATH, securityManager)
    , _fsPersistence(APSettings::read, APSettings::update, this, fs, AP_SETTINGS_FILE) {
}

void APSettingsService::begin() {
    _fsPersistence.readFromFS();
}

APNetworkStatus APSettingsService::getAPNetworkStatus() {
    return emsesp::EMSESP::network_.ap_connected() ? APNetworkStatus::ACTIVE : APNetworkStatus::INACTIVE;
}

void APSettings::read(const APSettings & settings, JsonObject root) {
    root["provision_mode"] = settings.provisionMode;
    root["ssid"]           = settings.ssid;
    root["password"]       = settings.password;
    root["channel"]        = settings.channel;
    root["ssid_hidden"]    = settings.ssidHidden;
    root["max_clients"]    = settings.maxClients;
    root["local_ip"]       = settings.localIP.toString();
    root["gateway_ip"]     = settings.gatewayIP.toString();
    root["subnet_mask"]    = settings.subnetMask.toString();
}

StateUpdateResult APSettings::update(JsonObject root, APSettings & settings) {
    APSettings newSettings{};
    newSettings.provisionMode = static_cast<uint8_t>(root["provision_mode"] | FACTORY_AP_PROVISION_MODE);

    switch (settings.provisionMode) {
    case AP_MODE_DISCONNECTED:
    case AP_MODE_NEVER:
        break;
    default:
        newSettings.provisionMode = AP_MODE_DISCONNECTED;
    }

    newSettings.ssid       = root["ssid"] | FACTORY_AP_SSID;
    newSettings.password   = root["password"] | FACTORY_AP_PASSWORD;
    newSettings.channel    = static_cast<uint8_t>(root["channel"] | FACTORY_AP_CHANNEL);
    newSettings.ssidHidden = root["ssid_hidden"] | FACTORY_AP_SSID_HIDDEN;
    newSettings.maxClients = static_cast<uint8_t>(root["max_clients"] | FACTORY_AP_MAX_CLIENTS);

    JsonUtils::readIP(root, "local_ip", newSettings.localIP, String(FACTORY_AP_LOCAL_IP));
    JsonUtils::readIP(root, "gateway_ip", newSettings.gatewayIP, String(FACTORY_AP_GATEWAY_IP));
    JsonUtils::readIP(root, "subnet_mask", newSettings.subnetMask, String(FACTORY_AP_SUBNET_MASK));

    if (newSettings == settings) {
        return StateUpdateResult::UNCHANGED;
    }

    settings = newSettings;

    // if the AP mode has changed, force a disconnect and reconnect
    if (settings.provisionMode != newSettings.provisionMode) {
        emsesp::EMSESP::network_.reconnect();
    }
    return StateUpdateResult::CHANGED;
}

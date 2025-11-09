#ifndef JsonUtils_h
#define JsonUtils_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <IPAddress.h>

#include "IPUtils.h"

class JsonUtils {
  public:
    static void readIP(JsonObject root, const char * key, IPAddress & ip, const char * def) {
        IPAddress defaultIp;
        if (!defaultIp.fromString(def)) {
            defaultIp = INADDR_NONE;
        }
        readIP(root, key, ip, defaultIp);
    }
    static void readIP(JsonObject root, const char * key, IPAddress & ip, const IPAddress & defaultIp = INADDR_NONE) {
        auto value = root[key];
        if (!value.is<String>() || !ip.fromString(value.as<String>())) {
            ip = defaultIp;
        }
    }
    static void readIP(JsonObjectConst root, const char * key, IPAddress & ip, const char * def) {
        IPAddress defaultIp;
        if (!defaultIp.fromString(def)) {
            defaultIp = INADDR_NONE;
        }
        readIP(root, key, ip, defaultIp);
    }
    static void readIP(JsonObjectConst root, const char * key, IPAddress & ip, const IPAddress & defaultIp = INADDR_NONE) {
        auto value = root[key];
        if (!value.is<String>() || !ip.fromString(value.as<String>())) {
            ip = defaultIp;
        }
    }
    static void writeIP(JsonObject root, const char * key, const IPAddress & ip) {
        if (IPUtils::isSet(ip)) {
            root[key] = ip.toString();
        }
    }
};

#endif

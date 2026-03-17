/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "espMqttClient.h"

#if defined(ARDUINO_ARCH_ESP32)
espMqttClient::espMqttClient(espMqttClientTypes::UseInternalTask useInternalTask)
    : MqttClientSetup(useInternalTask)
    , _client() {
    _transport = &_client;
}

espMqttClient::espMqttClient(uint8_t priority, uint8_t core)
    : MqttClientSetup(espMqttClientTypes::UseInternalTask::YES, priority, core)
    , _client() {
    _transport = &_client;
}

espMqttClientSecure::espMqttClientSecure(espMqttClientTypes::UseInternalTask useInternalTask)
    : MqttClientSetup(useInternalTask)
    , _client() {
    _transport = &_client;
}

espMqttClientSecure::espMqttClientSecure(uint8_t priority, uint8_t core)
    : MqttClientSetup(espMqttClientTypes::UseInternalTask::YES, priority, core)
    , _client() {
    _transport = &_client;
}

espMqttClientSecure & espMqttClientSecure::setInsecure() {
#ifndef NO_TLS_SUPPORT
    _client.client.setInsecure();
#endif
    return *this;
}

espMqttClientSecure & espMqttClientSecure::setCACert(const char * rootCA) {
#ifndef NO_TLS_SUPPORT
    _client.client.setCACert(rootCA);
#endif
    return *this;
}

espMqttClientSecure & espMqttClientSecure::setCertificate(const char * clientCa) {
#ifndef NO_TLS_SUPPORT
    _client.client.setCertificate(clientCa);
#endif
    return *this;
}

espMqttClientSecure & espMqttClientSecure::setPrivateKey(const char * privateKey) {
#ifndef NO_TLS_SUPPORT
    _client.client.setPrivateKey(privateKey);
#endif
    return *this;
}

espMqttClientSecure & espMqttClientSecure::setPreSharedKey(const char * pskIdent, const char * psKey) {
#ifndef NO_TLS_SUPPORT
#endif
    return *this;
}

#endif

#if defined(__linux__)
espMqttClient::espMqttClient()
    : MqttClientSetup(espMqttClientTypes::UseInternalTask::NO)
    , _client() {
    _transport = &_client;
}
#endif

/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <IPAddress.h>
#include "esp_tls.h"
#include "Transport.h"

namespace espMqttClientInternals {

class ClientSecureSync : public Transport {
  public:
    ClientSecureSync();
    ~ClientSecureSync();
    bool   connect(IPAddress ip, uint16_t port) override;
    bool   connect(const char * host, uint16_t port) override;
    size_t write(const uint8_t * buf, size_t size) override;
    int    read(uint8_t * buf, size_t size) override;
    void   stop() override;
    bool   connected() override;
    bool   disconnected() override;

    // TLS configuration (call before connect)
    void setCACert(const char * rootCA);
    void setCertificate(const char * clientCert);
    void setPrivateKey(const char * privateKey);
    void setPreSharedKey(const char * pskIdent, const char * psKey);
    void setInsecure();

  private:
    esp_tls_t *   _tls;
    esp_tls_cfg_t _cfg;
    bool          _connected;
#if defined(CONFIG_ESP_TLS_PSK_VERIFICATION)
    psk_hint_key_t _psk;
    unsigned char  _psk_key[64];
#endif
};

} // namespace espMqttClientInternals

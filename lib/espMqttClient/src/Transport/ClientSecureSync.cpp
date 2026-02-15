/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "ClientSecureSync.h"
#include "mbedtls_ssl.h" // triggers compilation of mbedtls SSL module (stripped from Tasmota libmbedtls.a)

#include <cstring>
#include <lwip/sockets.h>
#include <fcntl.h>

namespace espMqttClientInternals {

ClientSecureSync::ClientSecureSync()
    : _tls(nullptr)
    , _cfg{}
    , _connected(false) {
}

ClientSecureSync::~ClientSecureSync() {
    stop();
}

bool ClientSecureSync::connect(IPAddress ip, uint16_t port) {
    char host[16];
    sprintf(host, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    return connect(host, port);
}

bool ClientSecureSync::connect(const char * host, uint16_t port) {
    stop(); // clean up any previous connection

    _tls = esp_tls_init();
    if (!_tls) {
        return false;
    }

    if (esp_tls_conn_new_sync(host, strlen(host), port, &_cfg, _tls) <= 0) {
        esp_tls_conn_destroy(_tls);
        _tls = nullptr;
        return false;
    }

    _connected = true;

    // Set TCP_NODELAY and non-blocking mode on the underlying socket
    int fd = -1;
    if (esp_tls_get_conn_sockfd(_tls, &fd) == ESP_OK && fd >= 0) {
        int val = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
        // Make socket non-blocking so reads don't stall the MQTT event loop
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }
    }

    return true;
}

size_t ClientSecureSync::write(const uint8_t * buf, size_t size) {
    if (!_tls || !_connected) {
        return 0;
    }

    // Write all data, retrying on WANT_WRITE (non-blocking socket)
    size_t written = 0;
    while (written < size) {
        int ret = esp_tls_conn_write(_tls, buf + written, size - written);
        if (ret > 0) {
            written += ret;
        } else if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue; // retry
        } else {
            _connected = false;
            break;
        }
    }
    return written;
}

int ClientSecureSync::read(uint8_t * buf, size_t size) {
    if (!_tls || !_connected) {
        return -1;
    }

    int ret = esp_tls_conn_read(_tls, buf, size);
    if (ret > 0) {
        return ret;
    }
    if (ret == ESP_TLS_ERR_SSL_WANT_READ || ret == ESP_TLS_ERR_SSL_WANT_WRITE) {
        return -1; // no data available yet, still connected
    }
    // Connection closed or error
    _connected = false;
    return -1;
}

void ClientSecureSync::stop() {
    if (_tls) {
        esp_tls_conn_destroy(_tls);
        _tls = nullptr;
    }
    _connected = false;
}

bool ClientSecureSync::connected() {
    return _connected && _tls != nullptr;
}

bool ClientSecureSync::disconnected() {
    return !connected();
}

void ClientSecureSync::setCACert(const char * rootCA) {
    _cfg.cacert_pem_buf   = reinterpret_cast<const unsigned char *>(rootCA);
    _cfg.cacert_pem_bytes = strlen(rootCA) + 1;
}

void ClientSecureSync::setCertificate(const char * clientCert) {
    _cfg.clientcert_pem_buf   = reinterpret_cast<const unsigned char *>(clientCert);
    _cfg.clientcert_pem_bytes = strlen(clientCert) + 1;
}

void ClientSecureSync::setPrivateKey(const char * privateKey) {
    _cfg.clientkey_pem_buf   = reinterpret_cast<const unsigned char *>(privateKey);
    _cfg.clientkey_pem_bytes = strlen(privateKey) + 1;
}

void ClientSecureSync::setPreSharedKey(const char * pskIdent, const char * psKey) {
#if defined(CONFIG_ESP_TLS_PSK_VERIFICATION)
    _psk.hint      = pskIdent;
    size_t key_len = strlen(psKey) / 2;
    if (key_len > sizeof(_psk_key)) {
        key_len = sizeof(_psk_key);
    }
    for (size_t i = 0; i < key_len; i++) {
        sscanf(psKey + 2 * i, "%2hhx", &_psk_key[i]);
    }
    _psk.key          = _psk_key;
    _psk.key_size     = key_len;
    _cfg.psk_hint_key = &_psk;
#endif
}

void ClientSecureSync::setInsecure() {
    _cfg.cacert_pem_buf      = nullptr;
    _cfg.cacert_pem_bytes    = 0;
    _cfg.crt_bundle_attach   = nullptr;
    _cfg.use_global_ca_store = false;
    _cfg.skip_common_name    = true;
}

} // namespace espMqttClientInternals


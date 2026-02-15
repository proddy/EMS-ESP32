/*
 * Stub header to trigger PlatformIO Library Dependency Finder.
 *
 * The Tasmota Arduino platform ships a stripped libmbedtls.a that is missing
 * the core SSL/TLS implementation (mbedtls_ssl_read, mbedtls_ssl_write, etc.).
 * This library compiles the official mbedtls 3.6.5 SSL source files so that
 * esp_tls and other components that depend on mbedtls SSL can link.
 */
#pragma once

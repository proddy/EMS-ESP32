Import("env")
import os

# The Tasmota platform builds with CONFIG_MBEDTLS_TLS_DISABLED=y, stripping the
# SSL/TLS module from libmbedtls.a. We compile it from source and re-enable the
# necessary config macros for a minimal TLS 1.2 client.
try:
    platform = env.PioPlatform()
    framework_dir = platform.get_package_dir("framework-arduinoespressif32")
    mcu = env.BoardConfig().get("build.mcu", "esp32")
    base = os.path.join(framework_dir, "tools", "esp32-arduino-libs", mcu, "include", "mbedtls")

    paths = [
        os.path.join(base, "mbedtls", "library"),
        os.path.join(base, "mbedtls", "include"),
        os.path.join(base, "port", "include"),
    ]
    for p in paths:
        if os.path.isdir(p):
            env.Append(CPPPATH=[p])

    # Re-enable mbedtls TLS 1.2 client support (disabled by Tasmota sdkconfig)
    env.Append(CPPDEFINES=[
        # Core TLS
        "CONFIG_MBEDTLS_TLS_ENABLED",
        "CONFIG_MBEDTLS_TLS_CLIENT",
        "CONFIG_MBEDTLS_SSL_PROTO_TLS1_2",
        ("CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN", "16384"),
        # Key exchange methods (at least one required)
        "CONFIG_MBEDTLS_KEY_EXCHANGE_RSA",
        "CONFIG_MBEDTLS_KEY_EXCHANGE_ECDHE_RSA",
        "CONFIG_MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA",
        "CONFIG_MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA",
        "CONFIG_MBEDTLS_KEY_EXCHANGE_ECDH_RSA",
        # Optional but useful
        "CONFIG_MBEDTLS_SSL_RENEGOTIATION",
        "CONFIG_MBEDTLS_SSL_ALPN",
    ])
except Exception:
    pass

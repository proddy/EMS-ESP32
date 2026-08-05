# Changelog

For more details go to [emsesp.org](https://emsesp.org/).

## [3.9.0]

This release is based on the latest Espressif/Arduino core version 3. It brings in many memory and performance optimizations. Note it does require the user to manually migrate settings from 3.8.x to 3.9.0.

## Added

- user-requested LED blink [#3063](https://github.com/emsesp/EMS-ESP32/issues/3063)
- Commands Service that can be called via MQTT or API or used in the Scheduler Service
- option to disable factory reset [#3150](https://github.com/emsesp/EMS-ESP32/issues/3150)
- TLS support with 4MB boards without PSRAM

## Fixed

- shunting yard show json
- LED stayed off on a healthy system when "Disable LED" was unchecked
- Ethernet MAC address changed with the new SDK, breaking DHCP reservations

## Changed

- various memory optimizations [#3083](https://github.com/emsesp/EMS-ESP32/issues/3083)
- Scheduler name is now mandatory
- network fallback to AP only after start [#3090](https://github.com/emsesp/EMS-ESP32/issues/3090)
- replaced Web async-validator with custom validator and toast with native snackbar to reduce bundle size
- Gateway and Connect devices are shown in the Devices page, but disabled [3126](https://github.com/emsesp/EMS-ESP32/discussions/3126)
- show control setting only for master thermostats (0x10) [#3173](https://github.com/emsesp/EMS-ESP32/issues/3173)
- remove devices without entities not listed in 0x07 telegram


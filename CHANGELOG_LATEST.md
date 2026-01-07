# Changelog

For more details go to [emsesp.org](https://emsesp.org/).

## [3.8.1]

## Added

- update time saved in nvs
- heatpump entities [#2883](https://github.com/emsesp/EMS-ESP32/issues/2883)

## Fixed

- fix EMS bus disconnected errors on some systems [#2881](https://github.com/emsesp/EMS-ESP32/issues/2881)
- selflowtemp fix [#2876](https://github.com/emsesp/EMS-ESP32/issues/2876)
- updated valid GPIOs for ESP32S2, ESP32S3 and ESP32 that caused custom systems to block gpios [#2887](https://github.com/emsesp/EMS-ESP32/issues/2887)
- Junkers wwcharge offset [#2860](https://github.com/emsesp/EMS-ESP32/issues/2860)
- fixed minflowtemp [#2890](https://github.com/emsesp/EMS-ESP32/issues/2890)
- don't add HA uom/classes for bool values [#2885](https://github.com/emsesp/EMS-ESP32/issues/2885)
- fixed missing progress bar on web firmware uploads

## Changed

- snapshot gpios stored in temporary ram
- GPIOs stored along with the name and reported in log if conflicting
- free GPIOs depend on board profile [#2901](https://github.com/emsesp/EMS-ESP32/issues/2901)
- prefer PSram for mqtt queue [#2889](https://github.com/emsesp/EMS-ESP32/issues/2889)

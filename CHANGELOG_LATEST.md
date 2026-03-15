# Changelog

For more details go to [emsesp.org](https://emsesp.org/).

## [3.8.2]

## Added

- comfortpoint for BC400 [#2935](https://github.com/emsesp/EMS-ESP32/issues/2935)
- customize device brand [#2784](https://github.com/emsesp/EMS-ESP32/issues/2784)
- set model for ems-esp devices temperature, analog, etc. [#2958](https://github.com/emsesp/EMS-ESP32/discussions/2958)
- prometheus metrics for temperature/analog/scheduler/custom [#2962](https://github.com/emsesp/EMS-ESP32/issues/2962)
- boiler pumpkick [#2965](https://github.com/emsesp/EMS-ESP32/discussions/2965)
- heatpump reset [#2933](https://github.com/emsesp/EMS-ESP32/issues/2933)

## Fixed

- SRC climate creation [#2936](https://github.com/emsesp/EMS-ESP32/issues/2936) and [#2960](https://github.com/emsesp/EMS-ESP32/issues/2960)

## Changed

- weblogbuffer up to 1000 messages with PSRAM, mentioned in [#2933](https://github.com/emsesp/EMS-ESP32/issues/2933)
- validate custom entity writes, [#2931](https://github.com/emsesp/EMS-ESP32/issues/2931)
- remove wrong burnMinPower [#2918](https://github.com/emsesp/EMS-ESP32/issues/2918)
- store scheduler active state to nvs [#2946](https://github.com/emsesp/EMS-ESP32/discussions/2946)
- translated modes `heat` and `eco` for HA-climate mode-str-tpl
- support `minflowtemp` and `baseflowtemp` [#2969](https://github.com/emsesp/EMS-ESP32/discussions/2969)

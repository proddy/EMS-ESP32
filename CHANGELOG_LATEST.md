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
- 2.nd freshwater module (dhw4, dhw5) [#2991](https://github.com/emsesp/EMS-ESP32/issues/2991)
- full system backup and restore

## Fixed

- SRC climate creation [#2936](https://github.com/emsesp/EMS-ESP32/issues/2936) and [#2960](https://github.com/emsesp/EMS-ESP32/issues/2960)
- missing translations [#3015](https://github.com/emsesp/EMS-ESP32/issues/3015)

## Changed

- weblogbuffer up to 1000 messages with PSRAM, mentioned in [#2933](https://github.com/emsesp/EMS-ESP32/issues/2933)
- validate custom entity writes, [#2931](https://github.com/emsesp/EMS-ESP32/issues/2931)
- remove wrong burnMinPower [#2918](https://github.com/emsesp/EMS-ESP32/issues/2918)
- store scheduler active state to nvs [#2946](https://github.com/emsesp/EMS-ESP32/discussions/2946)
- translated modes `heat` and `eco` for HA-climate mode-str-tpl
- support `minflowtemp` and `baseflowtemp` [#2969](https://github.com/emsesp/EMS-ESP32/discussions/2969)
- update version if it is 00.00 in first read [#2981](https://github.com/emsesp/EMS-ESP32/issues/2981)
- device class for % values [#2980](https://github.com/emsesp/EMS-ESP32/issues/2980)
- fetch telegrams: set length to fetch [#3017](https://github.com/emsesp/EMS-ESP32/issues/3017)
- move http client from stack to heap
- heap optimizations [#3021](https://github.com/emsesp/EMS-ESP32/discussions/3021)

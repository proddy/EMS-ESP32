/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2025  emsesp.org - proddy, MichaelDvP
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EMSESP_SHOWER_H
#define EMSESP_SHOWER_H

#include "emsesp.h"

namespace emsesp {

class Shower {
  public:
    void start();
    void loop();

    void set_shower_state(bool state, bool force = false);
    void create_ha_discovery();

    void shower_timer(bool enable) {
        shower_timer_ = enable;
    }

    void shower_alert(bool enable) {
        shower_alert_ = enable;
    }

    void ha_reset() {
        ha_configdone_ = false;
    }

  private:
    static uuid::log::Logger logger_;

    static constexpr uint32_t SHOWER_PAUSE_TIME  = 15; // 15 seconds, max time if water is switched off & on during a shower
    static constexpr uint32_t SHOWER_OFFSET_TIME = 5;  // 5 seconds grace time, to calibrate actual time under the shower

    void shower_alert_start();
    void shower_alert_stop();

    // Configuration settings
    bool     shower_timer_          = false; // true if we want to report back on shower times
    bool     shower_alert_          = false; // true if we want the alert of cold water
    uint32_t shower_alert_trigger_  = 0;     // default 7 minutes, before trigger a shot of cold water
    uint32_t shower_alert_coldshot_ = 0;     // default 10 seconds for cold water before turning back hot water
    uint32_t shower_min_duration_   = 0;     // default 3 minutes (180 seconds), before recognizing its a shower
    uint32_t next_alert_            = 0;

    // State tracking
    bool ha_configdone_      = false; // for HA MQTT Discovery
    bool mqtt_sent_          = false; // track if initial MQTT state has been sent
    bool shower_state_       = false; // current shower active state
    bool old_shower_state_   = false; // previous state for change detection
    bool doing_cold_shot_    = false; // true if we've just sent a jolt of cold water
    bool force_coldshot_     = false; // flag to force a coldshot

    // Timing
    uint32_t timer_start_       = 0; // sec
    uint32_t timer_pause_       = 0; // sec
    uint32_t duration_          = 0; // sec
    uint32_t alert_timer_start_ = 0; // sec
};

} // namespace emsesp

#endif

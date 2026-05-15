/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020-2025  emsesp.org
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

#ifndef EMSESP_LED_H_
#define EMSESP_LED_H_

#include <Arduino.h>
#include <uuid/log.h>

#include "emsesp_common.h"

namespace emsesp {

class LED {
  public:
    enum Color { RED = 0, GREEN = 1, BLUE = 2, YELLOW = 3, OFF = 4, ON = 5 };

    void init();
    bool loop(uint8_t healthcheck, bool button_busy);

    void reset_led(bool default_state = true);   // turn the LED to default state or use false for off
    void start_led_fast_flash(uint8_t duration); // duration in seconds

    void set_led(Color color);
    void set_led_routine(std::string color, std::string pattern);

  private:
    static uuid::log::Logger logger_;

    void monitor(uint8_t led_routine, bool button_busy);
    void led_fast_flash();

    static constexpr uint32_t HEALTHCHECK_LED_LONG_DURATION       = 1000; // 1 second between flash sequences
    static constexpr uint32_t HEALTHCHECK_LED_LONG_FAST_DURATION  = 500;  // 1/2 second between flash sequences
    static constexpr uint32_t HEALTHCHECK_LED_FLASH_DURATION      = 150;  // 150ms
    static constexpr uint32_t HEALTHCHECK_LED_FLASH_FAST_DURATION = 150;
    static constexpr uint32_t LED_FLASH_INTERVAL_MS               = 100;  // LED toggle period during factory-reset flash
    static constexpr uint8_t  RGB_LED_BRIGHTNESS                  = 20;   // 255 is max brightness
    static constexpr uint8_t  LED_ON                              = HIGH; // LED on

    // local copies of the application settings
    uint8_t led_gpio_ = 0;
    uint8_t led_type_ = 0;
    bool    hide_led_ = false;

    bool     led_fast_flash_timer_ = false;
    uint32_t led_flash_start_time_ = 0;
    uint32_t led_flash_duration_   = 0;

    // led_flash() state
    bool     led_flash_state_  = false;
    uint32_t last_toggle_time_ = 0;

    // monitor() state
    bool     last_button_busy_ = false;
    uint32_t led_long_timer_   = 1; // 1 will kick it off immediately
    uint32_t led_short_timer_  = 0;
    uint8_t  led_flash_step_   = 0; // 0 means we're not in the short flash timer

    // set_led_routine() state
    Color color_steps_[3] = {Color::OFF, Color::OFF, Color::OFF};
};

} // namespace emsesp

#endif

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

#include "led.h"
#include "emsesp.h"

namespace emsesp {

uuid::log::Logger LED::logger_{F_(led), uuid::log::Facility::KERN};

// initialise the LED, fetching the settings from the WebSettingsService
// set the LED to on or off when in normal operating mode
void LED::init() {
    // copy the application settings
    EMSESP::webSettingsService.read([&](WebSettings & settings) {
        led_gpio_ = settings.led_gpio;
        led_type_ = settings.led_type;
        hide_led_ = settings.hide_led;
    });

    if (!led_gpio_) { // 0 means disabled
        LOG_INFO("LED disabled");
        return;
    }

    // for safety
    if (!led_type_) {
        pinMode(led_gpio_, OUTPUT);
    }

    reset_led(); // start with LED in default state, depending on if it's hidden or not
}

// handle LED routine
// called from the System::loop()
// returns true if the LED flash is active, i.e its a lock down state
bool LED::loop(uint8_t healthcheck, bool button_busy) {
    // if LED flashing is active it means its about to perform a factory reset, so don't do anything else and keep it flashing
    if (led_fast_flash_timer_) {
        led_fast_flash();
        return true;
    }

    // show the LED status based on the healthcheck and button busy status
    sequence_led(healthcheck, button_busy);

    return false;
}

// turn the LED back it's default state depending on if it's hidden or not
void LED::reset_led() {
    set_led(hide_led_ ? Color::OFF : Color::GREEN); // Green
}

// LED flash every few ms and then perform a factory reset
void LED::led_fast_flash() {
    uint32_t current_time = uuid::get_uptime();

    if (current_time - last_toggle_time_ >= LED_FLASH_INTERVAL_MS) {
        led_flash_state_  = !led_flash_state_;
        last_toggle_time_ = current_time;
        set_led(led_flash_state_ ? Color::YELLOW : Color::OFF); // Yellow
    }

    // after duration, turn off the LED
    if (current_time - led_flash_start_time_ >= led_flash_duration_) {
        set_led(Color::OFF);
        led_fast_flash_timer_ = false;
#ifndef EMSESP_DEBUG
        System::command_format(nullptr, 0); // Execute format operation, unless in debug mode
#endif
    }
}

// set LED on/off or RGB color
// ignores whether the LED is hidden or not (hide_led_ is set)
void LED::set_led(Color color) {
    uint8_t red   = 0;
    uint8_t green = 0;
    uint8_t blue  = 0;
    if (color == Color::RED) {
        red   = RGB_LED_BRIGHTNESS;
        green = 0;
        blue  = 0;
    } else if (color == Color::GREEN) {
        red   = 0;
        green = RGB_LED_BRIGHTNESS;
        blue  = 0;
    } else if (color == Color::BLUE) {
        red   = 0;
        green = 0;
        blue  = RGB_LED_BRIGHTNESS;
    } else if (color == Color::YELLOW) {
        red   = RGB_LED_BRIGHTNESS;
        green = RGB_LED_BRIGHTNESS;
        blue  = 0;
    } else if (color == Color::OFF) { // off
        red   = 0;
        green = 0;
        blue  = 0;
    } else if (color == Color::ON) { // white
        red   = RGB_LED_BRIGHTNESS;
        green = RGB_LED_BRIGHTNESS;
        blue  = RGB_LED_BRIGHTNESS;
    }

    if (!led_gpio_) {
        return;
    }

    if (led_type_) {
        rgbLedWrite(led_gpio_, red, green, blue);
    } else {
        digitalWrite(led_gpio_, (red == 0 && green == 0 && blue == 0) || color == Color::OFF ? !LED_ON : LED_ON);
    }
}

// set LED custom routine
// color is red, green, blue, yellow, white
// pattern is blink1, blink2, blink3, rgb
// For example: /api/system/led?data=red:blink1
// For older non-RGB models, the colour would default to just being on.
void LED::set_led_routine(std::string color, std::string pattern) {
    Color color_type = Color::OFF;
    if (color == "red") {
        color_type = Color::RED;
    } else if (color == "green") {
        color_type = Color::GREEN;
    } else if (color == "blue") {
        color_type = Color::BLUE;
    } else if (color == "yellow") {
        color_type = Color::YELLOW;
    } else if (color == "white") {
        color_type = Color::ON;
    } else {
        color_type = Color::OFF;
    }

    color_steps_[0] = Color::OFF;
    color_steps_[1] = Color::OFF;
    color_steps_[2] = Color::OFF;

    if (pattern == "blink1") {
        color_steps_[0] = color_type;
    } else if (pattern == "blink2") {
        color_steps_[0] = color_type;
        color_steps_[1] = color_type;
    } else if (pattern == "blink3") {
        color_steps_[0] = color_type;
        color_steps_[1] = color_type;
        color_steps_[2] = color_type;
    } else if (pattern == "rgb") {
        color_steps_[0] = Color::RED;
        color_steps_[1] = Color::GREEN;
        color_steps_[2] = Color::BLUE;
    }

    // when this is called we want the sequence_led to restart immediately and skip the long pause
    led_long_timer_ = uuid::get_uptime() + HEALTHCHECK_LED_FLASH_FAST_DURATION + 200UL;
}

// uses LED to show system health and user-requested LED blinks
// it works in a batch of 3 configured flashes, then a long pause
// the timing is different for user-requested LED blink and for system healthcheck
void LED::sequence_led(uint8_t healthcheck, bool button_busy) {
    // see if we're doing as user-requested LED blink
    bool is_user_led_blink = false;
    if (color_steps_[0] != Color::OFF || color_steps_[1] != Color::OFF || color_steps_[2] != Color::OFF) {
        is_user_led_blink = true;
    }

    // if button is pressed, show LED (yellow on RGB LED, on/off on standard LED) and exit
    if (last_button_busy_ != button_busy) {
        last_button_busy_ = button_busy;
        set_led(button_busy ? Color::OFF : Color::YELLOW); // Yellow
    }

    // we only need to run the LED sequence_led if there are errors, or if a button has been pressed or a user-requested LED blink is active
    if ((!healthcheck || button_busy) && !is_user_led_blink) {
        return; // nothing to show
    }

    // first long pause before we start flashing
    auto current_time = uuid::get_uptime();
    if (led_long_timer_
        && (uint32_t)(current_time - led_long_timer_) >= (is_user_led_blink ? HEALTHCHECK_LED_LONG_FAST_DURATION : HEALTHCHECK_LED_LONG_DURATION)) {
        led_short_timer_ = current_time; // start the short timer
        led_long_timer_  = 0;            // stop long timer
        led_flash_step_  = 1;            // enable the short flash timer
    }

    // the flash timer which starts after the long pause
    if (led_flash_step_
        && (uint32_t)(current_time - led_short_timer_) >= (is_user_led_blink ? HEALTHCHECK_LED_FLASH_FAST_DURATION : HEALTHCHECK_LED_FLASH_DURATION)) {
        led_long_timer_  = 0; // stop the long timer
        led_short_timer_ = current_time;

        if (++led_flash_step_ == 8) {
            // finished first iteration, reset the whole sequence, turn off LED
            led_long_timer_ = uuid::get_uptime();
            led_flash_step_ = 0;
            set_led(Color::OFF);
        } else if (led_flash_step_ % 2) {
            // handle the three step events (on odd numbers 3,5,7 etc). see if we need to set a LED color

            // For the system healthcheck:
            //  1 flash (blue) is the EMS bus is not connected
            //  2 flashes (red, red) if the network (wifi or ethernet) is not connected
            //  3 flashes (red, red, blue) is both the bus and the network are not connected
            bool no_network = (healthcheck & System::HEALTHCHECK_NO_NETWORK) == System::HEALTHCHECK_NO_NETWORK;
            bool no_bus     = (healthcheck & System::HEALTHCHECK_NO_BUS) == System::HEALTHCHECK_NO_BUS;

            switch (led_flash_step_) {
            case 3: // first flash
                if (is_user_led_blink) {
                    set_led(color_steps_[0]);
                    color_steps_[0] = Color::OFF; // reset
                } else {
                    if (no_network) {
                        set_led(Color::RED); // red, no network
                    } else if (no_bus) {
                        set_led(Color::BLUE); // blue, no bus
                    }
                }
                break;
            case 5: // second flash
                if (is_user_led_blink) {
                    set_led(color_steps_[1]);
                    color_steps_[1] = Color::OFF; // reset
                } else if (no_network) {
                    set_led(Color::RED); // red, no network
                }
                break;
            case 7: // third flash
                if (is_user_led_blink) {
                    set_led(color_steps_[2]);
                    color_steps_[2] = Color::OFF; // reset
                } else if (no_network && no_bus) {
                    set_led(Color::BLUE); // blue, no network and no bus
                }
                break;
            default:
                break;
            }
        } else {
            // turn off LED after the LED flash, or on even number count
            set_led(Color::OFF);
        }
    }
}

// Start the LED flash timer - duration in seconds
void LED::start_led_fast_flash(uint8_t duration) {
    // Don't start if already running
    if (led_fast_flash_timer_) {
        return;
    }

    // Reset counter and state
    led_flash_start_time_ = uuid::get_uptime();        // current time
    led_flash_duration_   = (uint32_t)duration * 1000; // duration in milliseconds
    led_fast_flash_timer_ = true;                      // it's active
}

} // namespace emsesp

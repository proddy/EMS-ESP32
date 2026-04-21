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

#ifndef EMSESP_OLED_H
#define EMSESP_OLED_H

#include <Adafruit_GFX.h>
#include <SSD1306.h>

#include "emsesp.h"

namespace emsesp {

class OLED {
  public:
    OLED();

    void start();
    void loop();

    void advanceLine();
    void updateDisplay();
    void showMenu();

  private:
    static uuid::log::Logger logger_;

    SSD1306 display_; // display driver

    // settings based on an OLD 128x32 with normal 6x8 font
    static constexpr uint8_t  SCREEN_WIDTH          = 128; // OLED display width, in pixels
    static constexpr uint8_t  SCREEN_HEIGHT         = 32;  // OLED display height, in pixels
    static constexpr int8_t   OLED_RESET            = -1;  // Reset pin # (or -1 if sharing MCU reset)
    static constexpr uint8_t  SCREEN_ADDRESS        = 0x3C;
    static constexpr uint32_t CLEAR_INTERVAL_MS     = 10000; // 10 seconds before clearing the screen
    static constexpr uint8_t  SDA_PIN               = 12;
    static constexpr uint8_t  SCL_PIN               = 13;
    static constexpr uint32_t SCROLL_INTERVAL_MS    = 1000; // Auto-scroll every 1 second
    static constexpr uint8_t  VISIBLE_LINES         = 2;    // number of lines to display at once in the body
    static constexpr uint8_t  VISIBLE_CHARS_IN_LINE = 21;   // max number of characters in a row/line
    static constexpr uint8_t  BODY_Y_START          = 13;   // pixel y-pos where body text starts (below header line)
    static constexpr uint8_t  LINE_HEIGHT_PX        = 9;    // 8px font + 1px gap
    static constexpr uint8_t  MAX_LINES             = 12;   // max number of lines in page
    static constexpr uint8_t  TOTAL_PAGES           = 4;

    bool initialized_ = false;

    static uint32_t last_clear_;
    static uint8_t  current_page_;
    static uint32_t lastScrollTime_;
    static uint8_t  scrollLine_;
    static uint8_t  totalLines_;
    static bool     display_blank_; // true after boot or inactivity clear
};

} // namespace emsesp

#endif

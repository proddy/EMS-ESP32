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

#include "oled.h"

#ifndef EMSESP_STANDALONE
#include <Wire.h>
#endif

namespace emsesp {

uuid::log::Logger OLED::logger_{F_(oled), uuid::log::Facility::CONSOLE};

uint32_t OLED::last_clear_     = 0;
uint8_t  OLED::current_page_   = 0;
uint32_t OLED::lastScrollTime_ = 0;
uint8_t  OLED::scrollLine_     = 0;
uint8_t  OLED::totalLines_     = 0;
bool     OLED::display_blank_  = false;

OLED::OLED()
    : display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

void OLED::start() {
    Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C SDA/SCL pins
    LOG_INFO("Initialized OLED display");

    if (!display_.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        LOG_ERROR("SSD1306 allocation failed");
        initialized_ = false;
        return;
    }
    initialized_ = true;

    // Show splash screen - will automatically clear display after 3 seconds
    display_.display();
    last_clear_ = uuid::get_uptime_ms() + (CLEAR_INTERVAL_MS - 3000);
}

// called when button is pressed in System::button_OnClick()
// first click after boot or after the inactivity blank: just wake up on page 0.
// subsequent clicks advance to the next page.
void OLED::showMenu() {
    if (!display_blank_) {
        current_page_ = (current_page_ + 1) % TOTAL_PAGES;
    }
    scrollLine_     = 0;
    lastScrollTime_ = millis();              // give the user the full interval to read
    last_clear_     = uuid::get_uptime_ms(); // (re)start the 10s blank timer
    updateDisplay();
}

void OLED::loop() {
    // if screen is not blank then clear display automatically after 10 seconds
    if (!display_blank_ && uuid::get_uptime_ms() - last_clear_ > CLEAR_INTERVAL_MS) {
        display_.clearDisplay();
        display_.display();
        current_page_  = 0; // reset to first page
        scrollLine_    = 0;
        last_clear_    = uuid::get_uptime_ms();
        display_blank_ = true;
        return;
    }

    // automatic scroll (only while the menu is showing)
    if (!display_blank_ && totalLines_ > 0 && millis() - lastScrollTime_ > SCROLL_INTERVAL_MS) {
        advanceLine();
    }
}

// handle rendering of the screen content, scrolling through the lines
void OLED::advanceLine() {
    // Once we've scrolled past the last line of this page, keep on same page
    if (totalLines_ > 0) {
        scrollLine_++;
        if (scrollLine_ >= totalLines_) {
            scrollLine_ = 0;
        }
    }
    lastScrollTime_ = millis(); // Reset the timer
    updateDisplay();
}

// Content - dynamically build the logs for this page
// first line is the header
void OLED::updateDisplay() {
    char logs[MAX_LINES][20] = {}; // zero-init for empty slots
    switch (current_page_) {
    case 0: {
        // shows version, system status, hostname
        strcpy(logs[0], "EMS-ESP");
        sprintf(logs[1], "Version: %s", EMSESP_APP_VERSION);
        auto         system_status = EMSESP::system_.systemStatus();
        const char * status_str;
        switch (system_status) {
        case SYSTEM_STATUS::SYSTEM_STATUS_PENDING_UPLOAD:
        case SYSTEM_STATUS::SYSTEM_STATUS_UPLOADING:
            status_str = "UPLOADING";
            break;
        case SYSTEM_STATUS::SYSTEM_STATUS_ERROR_UPLOAD:
            status_str = "ERROR UPLOAD";
            break;
        case SYSTEM_STATUS::SYSTEM_STATUS_PENDING_RESTART:
        case SYSTEM_STATUS::SYSTEM_STATUS_RESTART_REQUESTED:
            status_str = "RESTARTING";
            break;
        case SYSTEM_STATUS::SYSTEM_STATUS_INVALID_GPIO:
            status_str = "INVALID GPIO";
            break;
        default:
            status_str = "OK";
            break;
        }
        sprintf(logs[2], "Status: %s", status_str);
        sprintf(logs[3], "Hostname: %s", EMSESP::system_.hostname().c_str());
        sprintf(logs[4], "Model: %s", EMSESP::system_.getBBQKeesGatewayDetails().c_str());
        break;
    }
    case 1:
        // shows uptime, free mem, free psram
        strcpy(logs[0], "SYSTEM");
        sprintf(logs[1], "Uptime: %s", uuid::log::format_timestamp_ms(uuid::get_uptime_ms(), 3).c_str());
        sprintf(logs[3], "Free Mem: %d KB", ESP.getFreeHeap() / 1024);
        sprintf(logs[4], "Free PSRAM: %d KB", ESP.getFreePsram() / 1024);
        break;
    case 2:
        // shows network ip, mqtt status, ntp status, ap status, syslog status
        strcpy(logs[0], "SERVICES");
        if (EMSESP::system_.ethernet_connected()) {
            sprintf(logs[1], "IP: %s", uuid::printable_to_string(ETH.localIP()).c_str());
        } else if (WiFi.status() == WL_CONNECTED) {
            sprintf(logs[1], "IP: %s", uuid::printable_to_string(WiFi.localIP()).c_str());
        }
        sprintf(logs[2], "MQTT: %s", EMSESP::mqtt_.connected() ? "CONNECTED" : "DISCONNECTED");
        sprintf(logs[3], "NTP: %s", (EMSESP::system_.ntp_connected() == 1) ? "CONNECTED" : "DISCONNECTED");
        sprintf(logs[4], "AP: %s", EMSESP::esp32React.apStatus() == 1 ? "CONNECTED" : "DISCONNECTED");
        sprintf(logs[5], "Syslog: %s", EMSESP::system_.syslog_enabled() ? "ENABLED" : "DISABLED");
        break;
    case 3:
        // shows EMS bus traffic, number of ems devices, Rx Reads, Rx Fails, Tx Reads, Tx Writes, Tx Fails
        strcpy(logs[0], "EMS BUS");
        sprintf(logs[1], "Uptime: %d sec", EMSbus::bus_uptime());
        sprintf(logs[1], "Devices: %d", EMSESP::count_devices());
        sprintf(logs[2], "Rx Reads: %d", EMSESP::rxservice_.telegram_count());
        sprintf(logs[3], "Rx Fails: %d", EMSESP::rxservice_.telegram_error_count());
        sprintf(logs[4], "Tx Reads: %d", EMSESP::txservice_.telegram_read_count());
        sprintf(logs[5], "Tx Writes: %d", EMSESP::txservice_.telegram_write_count());
        sprintf(logs[6], "Tx Fails: %d", EMSESP::txservice_.telegram_read_fail_count() + EMSESP::txservice_.telegram_write_fail_count());
    }

    // prepare the header text, with page number and header line
    char header[20];
    sprintf(header, "%s (%d/%d)", logs[0], current_page_ + 1, TOTAL_PAGES);

    //  render
    display_.clearDisplay();
    display_.setTextColor(SSD1306_WHITE);
    display_.setTextSize(1);     // 1 is default 6x8, 2 is 12x16, 3 is 18x24
    display_.setTextWrap(false); // Disables word wrap
    display_.setCursor(0, 0);
    display_.println(header);
    display_.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // count how many lines are in the content for this page
    totalLines_ = 0;
    for (int i = 1; i < MAX_LINES; i++) {
        if (logs[i][0] != '\0') {
            totalLines_++;
        }
    }

    // draw up to VISIBLE_LINES starting from scrollLine_
    if (totalLines_ > 0) {
        if (scrollLine_ >= totalLines_) {
            scrollLine_ = 0; // safety in case page changed to a shorter one
        }
        uint8_t yPos  = BODY_Y_START;
        uint8_t count = (totalLines_ < VISIBLE_LINES) ? totalLines_ : VISIBLE_LINES;
        for (uint8_t i = 0; i < count; i++) {
            uint8_t lineIndex = scrollLine_ + i;
            if (lineIndex >= totalLines_) {
                // Don't print extra blank line, just stop rendering
                break;
            }
            display_.setCursor(0, yPos);
            // Don't print blank/log lines, just in case
            if (logs[lineIndex + 1][0] != '\0') {
                display_.print(logs[lineIndex + 1]);
            }
            yPos += LINE_HEIGHT_PX;
        }
    }

    // vertical scroll bar on the right edge: shows how far through the list we are. Only drawn when there are more lines than fit on screen.
    if (totalLines_ > VISIBLE_LINES) {
        constexpr uint8_t TRACK_TOP = BODY_Y_START - 1;          // just below the header rule
        constexpr uint8_t TRACK_H   = SCREEN_HEIGHT - TRACK_TOP; // to the very bottom
        constexpr uint8_t TRACK_X   = SCREEN_WIDTH - 1;          // rightmost column

        // Thumb: height proportional to visible/total, position to scrollLine_/total.
        uint8_t thumbH = (TRACK_H * VISIBLE_LINES) / totalLines_;
        if (thumbH < 2) {
            thumbH = 2;
        }
        uint8_t thumbY = TRACK_TOP + ((TRACK_H - thumbH) * scrollLine_) / (totalLines_ - 1);
        display_.fillRect(TRACK_X - 2, thumbY, 3, thumbH, SSD1306_WHITE);
    }

    display_.display();
    display_blank_ = false;
}
} // namespace emsesp

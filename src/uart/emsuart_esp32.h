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

/*
 * ESP32 UART port by @ArwedL and improved by @MichaelDvP. See https://github.com/emsesp/EMS-ESP/issues/380
 */

#pragma once

#include <cstdint>

// FreeRTOS task configuration
#ifndef EMSESP_UART_RUNNING_CORE
#define EMSESP_UART_RUNNING_CORE -1
#endif

#ifndef EMSESP_UART_STACKSIZE
#define EMSESP_UART_STACKSIZE 2560
#endif

#ifndef EMSESP_UART_PRIORITY
#define EMSESP_UART_PRIORITY configMAX_PRIORITIES - 1
#endif

// UART hardware configuration
#define EMSUART_NUM UART_NUM_1  // on C3 and S2 there is no UART2, use UART1 for all
#define EMSUART_BAUD 9600       // uart baud rate for the EMS circuit

namespace emsesp {

// EMS buffer configuration
inline constexpr uint8_t EMS_MAXBUFFERSIZE = 33;  // max size of the buffer. EMS packets are max 32 bytes, plus extra for BRK

// Transmission modes
enum class TxMode : uint8_t {
    EMS1    = 1,  // EMS 1.0 mode with echo detection
    EMSPLUS = 2,  // EMS+ protocol with extended delays
    HT3     = 3,  // HT3/Junkers protocol with 7-bit delays
    HW      = 4   // Hardware-controlled mode
};

// Transmission status codes
enum class TxStatus : uint8_t {
    ERR = 0,  // Transmission error
    OK  = 1   // Transmission successful
};

// Timing constants (in microseconds)
namespace Timing {
    // Base timing - bit time at 9600 baud
    inline constexpr uint32_t BIT_TIME = 104;  // microseconds per bit at 9600 baud

    // EMS 1.0 / Default mode timing
    inline constexpr uint32_t EMS_BUSY_WAIT = BIT_TIME / 8;                      // 13 µs - polling interval for echo detection
    inline constexpr uint32_t EMS_TIMEOUT   = 20 * BIT_TIME / EMS_BUSY_WAIT;     // timeout counter for echo detection
    inline constexpr uint32_t EMS_BRK       = BIT_TIME * 10;                     // 1040 µs - break signal duration

    // HT3/Junkers protocol timing
    // Time to send one byte (10 bits) plus 7-bit delay
    inline constexpr uint32_t HT3_WAIT = BIT_TIME * 17;  // 1768 µs - inter-byte delay
    inline constexpr uint32_t HT3_BRK  = BIT_TIME * 11;  // 1144 µs - break signal duration

    // EMS+ protocol timing
    // Time to send one byte (10 bits) plus one additional byte time delay
    inline constexpr uint32_t EMSPLUS_WAIT = BIT_TIME * 20;  // 2080 µs - inter-byte delay
    inline constexpr uint32_t EMSPLUS_BRK  = BIT_TIME * 11;  // 1144 µs - break signal duration
} // namespace Timing

class EMSuart {
  public:
    EMSuart()  = default;
    ~EMSuart() = default;

    // Disable copy and move
    EMSuart(const EMSuart &)            = delete;
    EMSuart & operator=(const EMSuart &) = delete;
    EMSuart(EMSuart &&)                 = delete;
    EMSuart & operator=(EMSuart &&)      = delete;

    static void     start(uint8_t tx_mode, uint8_t rx_gpio, uint8_t tx_gpio);
    static void     send_poll(uint8_t data);
    static void     stop();
    static TxStatus transmit(const uint8_t * buf, uint8_t len);
    
    static uint8_t last_tx_src() {
        return last_tx_src_;
    }

  private:
    static void IRAM_ATTR uart_gen_break(uint32_t length_us);
    static void           uart_event_task(void * pvParameters);
    static void           transmit_with_delay(const uint8_t * buf, uint8_t len, uint32_t delay_us, uint32_t break_us);
    static uint8_t        last_tx_src_;
};

} // namespace emsesp

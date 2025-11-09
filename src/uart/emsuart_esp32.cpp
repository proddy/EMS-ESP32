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

#ifndef EMSESP_STANDALONE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "soc/uart_reg.h"
#include "uart/emsuart_esp32.h"
#include "emsesp.h"

namespace emsesp {
uint8_t EMSuart::last_tx_src_ = 0;

static TaskHandle_t  xHandle;
static QueueHandle_t uart_queue;
uint8_t              tx_mode_     = 0xFF;
uint32_t             inverse_mask = 0;

constexpr uint8_t HW_BREAK_DURATION = 10; // break duration in bit times for HW mode

/*
* receive task, wait for break and call incoming_telegram
*/
void EMSuart::uart_event_task(void * pvParameters) {
    uart_event_t event;
    uint8_t      telegram[EMS_MAXBUFFERSIZE];
    uint8_t      length = 0;

    while (1) {
        //Waiting for UART event.
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
            if (event.type == UART_DATA) {
                length += event.size;
            } else if (event.type == UART_BREAK) {
                if (length == 2 || (length >= 6 && length <= EMS_MAXBUFFERSIZE)) {
                    uart_read_bytes(EMSUART_NUM, telegram, length, portMAX_DELAY);
                    EMSESP::incoming_telegram(telegram, (uint8_t)(length - 1));
                } else { // flush buffer up to break
                    uart_flush_input(EMSUART_NUM);
                }
                length = 0;
            } else if (event.type == UART_BUFFER_FULL) {
                uart_flush_input(EMSUART_NUM);
                length = 0;
            }
        }
    }
    vTaskDelete(NULL);
}

/*
 * init UART driver
 */
void EMSuart::start(const uint8_t tx_mode, const uint8_t rx_gpio, const uint8_t tx_gpio) {
    if (tx_mode_ == 0xFF) {
        uart_config_t uart_config = {.baud_rate           = EMSUART_BAUD,
                                     .data_bits           = UART_DATA_8_BITS,
                                     .parity              = UART_PARITY_DISABLE,
                                     .stop_bits           = UART_STOP_BITS_1,
                                     .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
                                     .rx_flow_ctrl_thresh = 0,
                                     .source_clk          = UART_SCLK_APB
#if ESP_ARDUINO_VERSION_MAJOR >= 3
                                     ,
                                     .flags = {0, 0}
#endif
        };
#if defined(EMSUART_RX_INVERT)
        inverse_mask |= UART_SIGNAL_RXD_INV;
#endif
#if defined(EMSUART_TX_INVERT)
        inverse_mask |= UART_SIGNAL_TXD_INV;
#endif
        uart_param_config(EMSUART_NUM, &uart_config);
        uart_set_pin(EMSUART_NUM, tx_gpio, rx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_set_line_inverse(EMSUART_NUM, inverse_mask);
        uart_driver_install(EMSUART_NUM, UART_FIFO_LEN + 1, 0, (EMS_MAXBUFFERSIZE + 1) * 2, &uart_queue, 0); // buffer must be > fifo
        uart_set_rx_full_threshold(EMSUART_NUM, 1);
        uart_set_rx_timeout(EMSUART_NUM, 0); // disable

        // note esp32s3 crashes with 2k stacksize, stack overflow here sometimes wipes settingsfiles.
#if defined(CONFIG_FREERTOS_UNICORE) || (EMSESP_UART_RUNNING_CORE < 0)
        xTaskCreate(uart_event_task, "uart_event_task", EMSESP_UART_STACKSIZE, NULL, EMSESP_UART_PRIORITY, &xHandle);
#else
        xTaskCreatePinnedToCore(uart_event_task, "uart_event_task", EMSESP_UART_STACKSIZE, NULL, EMSESP_UART_PRIORITY, &xHandle, EMSESP_UART_RUNNING_CORE);
#endif
    } else {
        vTaskResume(xHandle);
    }
    tx_mode_ = tx_mode;
    uart_enable_intr_mask(EMSUART_NUM, UART_BRK_DET_INT_ENA | UART_RXFIFO_FULL_INT_ENA);
}

/*
 * Stop, disable interrupt
 */
void EMSuart::stop() {
    if (tx_mode_ != 0xFF) { // only call after driver initialisation
        uart_disable_intr_mask(EMSUART_NUM, UART_BRK_DET_INT_ENA | UART_RXFIFO_FULL_INT_ENA);
        vTaskSuspend(xHandle);
    }
};

/*
 * generate <BRK> by inverting tx
 */
void IRAM_ATTR EMSuart::uart_gen_break(uint32_t length_us) {
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    uart_set_line_inverse(EMSUART_NUM, UART_SIGNAL_TXD_INV ^ inverse_mask);
    delayMicroseconds(length_us);
    uart_set_line_inverse(EMSUART_NUM, inverse_mask);
    portEXIT_CRITICAL(&mux);
}

/*
 * Sends a 1-byte poll, ending with a <BRK>
 */
void EMSuart::send_poll(const uint8_t data) {
    transmit(&data, 1);
}

/*
 * Helper function for timed byte transmission
 */
void EMSuart::transmit_with_delay(const uint8_t * buf, const uint8_t len, const uint32_t delay_us, const uint32_t break_us) {
    for (uint8_t i = 0; i < len; i++) {
        uart_write_bytes(EMSUART_NUM, &buf[i], 1);
        delayMicroseconds(delay_us);
    }
    uart_gen_break(break_us);
}

/*
 * Send data to Tx line, ending with a <BRK>
 * buf contains the CRC and len is #bytes including the CRC
 * returns TxStatus code
 */
TxStatus EMSuart::transmit(const uint8_t * buf, const uint8_t len) {
    if (len == 0 || len >= EMS_MAXBUFFERSIZE) {
        return TxStatus::ERR;
    }

    if (tx_mode_ == 0) {
        return TxStatus::OK;
    }

    last_tx_src_ = len < 4 ? 0 : buf[0];

    const uint8_t hw_mode      = static_cast<uint8_t>(TxMode::HW);
    const uint8_t emsplus_mode = static_cast<uint8_t>(TxMode::EMSPLUS);
    const uint8_t ht3_mode     = static_cast<uint8_t>(TxMode::HT3);

    if (tx_mode_ == hw_mode) { // hardware controlled mode
        uart_write_bytes_with_break(EMSUART_NUM, buf, len, HW_BREAK_DURATION);
    } else if (tx_mode_ == emsplus_mode) { // EMS+ with long delay
        transmit_with_delay(buf, len, Timing::EMSPLUS_WAIT, Timing::EMSPLUS_BRK);
    } else if (tx_mode_ == ht3_mode) { // HT3 with 7 bittimes delay
        transmit_with_delay(buf, len, Timing::HT3_WAIT, Timing::HT3_BRK);
    } else { // mode DEFAULT: wait for echo after each byte
        for (uint8_t i = 0; i < len; i++) {
            size_t rx_len;
            uart_get_buffered_data_len(EMSUART_NUM, &rx_len);
            uart_write_bytes(EMSUART_NUM, &buf[i], 1);
            uint16_t timeoutcnt = Timing::EMS_TIMEOUT;
            size_t   new_rx_len;
            do {
                delayMicroseconds(Timing::EMS_BUSY_WAIT);
                uart_get_buffered_data_len(EMSUART_NUM, &new_rx_len);
            } while ((new_rx_len == rx_len) && (--timeoutcnt));
        }
        uart_gen_break(Timing::EMS_BRK);
    }
    return TxStatus::OK;
}

} // namespace emsesp

#endif

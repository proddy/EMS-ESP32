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

#include "roomcontrol.h"

namespace emsesp {

// init statics
bool     Roomctrl::switch_off_[HCS]   = {false, false, false, false};
uint32_t Roomctrl::send_time_[HCS]    = {0, 0, 0, 0};
uint32_t Roomctrl::receive_time_[HCS] = {0, 0, 0, 0};
int16_t  Roomctrl::remotetemp_[HCS]   = {EMS_VALUE_INT16_NOTSET, EMS_VALUE_INT16_NOTSET, EMS_VALUE_INT16_NOTSET, EMS_VALUE_INT16_NOTSET};
uint8_t  Roomctrl::remotehum_[HCS]    = {EMS_VALUE_UINT8_NOTSET, EMS_VALUE_UINT8_NOTSET, EMS_VALUE_UINT8_NOTSET, EMS_VALUE_UINT8_NOTSET};
uint8_t  Roomctrl::sendtype_[HCS]     = {SendType::TEMP, SendType::TEMP, SendType::TEMP, SendType::TEMP};
uint8_t  Roomctrl::type_[HCS]         = {RemoteType::NONE, RemoteType::NONE, RemoteType::NONE, RemoteType::NONE};
uint32_t Roomctrl::timeout_           = 0;

/**
 * set the temperature,
 */
void Roomctrl::set_timeout(uint8_t t) {
    timeout_ = t * 3600000; // ms
}
void Roomctrl::set_remotetemp(const uint8_t type, const uint8_t hc, const int16_t temp) {
    if (!type_[hc] && !type) {
        return;
    }
    if (remotetemp_[hc] != EMS_VALUE_INT16_NOTSET && temp == EMS_VALUE_INT16_NOTSET) { // switch remote off
        remotetemp_[hc] = EMS_VALUE_INT16_NOTSET;
        switch_off_[hc] = true;
        send_time_[hc]  = uuid::get_uptime() - SEND_INTERVAL; // send now
        sendtype_[hc]   = SendType::TEMP;
        return;
    }
    if (hc >= HCS || !type) {
        return;
    }
    if (remotetemp_[hc] != temp) {
        send_time_[hc] = uuid::get_uptime() - SEND_INTERVAL; // send now
        sendtype_[hc]  = SendType::TEMP;
    }
    type_[hc]         = type;
    remotetemp_[hc]   = temp;
    receive_time_[hc] = uuid::get_uptime();
}

// set humidity for RC100H emulation
void Roomctrl::set_remotehum(const uint8_t type, const uint8_t hc, const int8_t hum) {
    if (hc >= HCS || type != type_[hc]) {
        return;
    }
    if (remotehum_[hc] != hum) {
        send_time_[hc] = uuid::get_uptime() - SEND_INTERVAL; // send now
        sendtype_[hc]  = SendType::HUMI;
    }
    remotehum_[hc] = hum;
}

uint8_t Roomctrl::get_hc(const uint8_t addr) {
    const uint8_t masked_addr = addr & 0x7F;
    
    // SENSOR range (0x40-0x44)
    if (masked_addr >= 0x40 && masked_addr <= 0x44) {
        const uint8_t hc = masked_addr - 0x40;
        if (type_[hc] == SENSOR) {
            return hc;
        }
    }
    // RC100H, RC200, RC100, RT800 range (0x38-0x3B)
    else if (masked_addr >= 0x38 && masked_addr <= 0x3B) {
        const uint8_t hc   = masked_addr - 0x38;
        const uint8_t type = type_[hc];
        if (type == RC100H || type == RC200 || type == RC100 || type == RT800) {
            return hc;
        }
    }
    // RC20, FB10 range (0x18-0x1B)
    else if (masked_addr >= 0x18 && masked_addr <= 0x1B) {
        const uint8_t hc   = masked_addr - 0x18;
        const uint8_t type = type_[hc];
        if (type == RC20 || type == FB10) {
            return hc;
        }
    }
    
    return 0xFF; // invalid
}

/**
 * if remote control is active send the temperature every 15 seconds
 */
void Roomctrl::send(const uint8_t addr) {
    // Quick exit for invalid addresses
    if (addr & 0x80) {
        return;
    }
    
    const uint8_t hc = get_hc(addr);
    // check address, reply only on addresses 0x18..0x1B or 0x40..0x43
    if (hc >= HCS) {
        return;
    }
    
    // no reply if the temperature is not set
    if (!switch_off_[hc] && remotetemp_[hc] == EMS_VALUE_INT16_NOTSET && remotehum_[hc] == EMS_VALUE_UINT8_NOTSET) {
        return;
    }

    // Check for timeout
    if (!switch_off_[hc] && timeout_ && (uuid::get_uptime() - receive_time_[hc]) > timeout_) {
        remotetemp_[hc] = EMS_VALUE_INT16_NOTSET;
        switch_off_[hc] = true;
        sendtype_[hc]   = SendType::TEMP;
        EMSESP::logger().warning("remotetemp timeout hc%d, stop sending roomtemperature to thermostat", hc);
    }
    
    // Check send interval
    if (switch_off_[hc] || (uuid::get_uptime() - send_time_[hc]) > SEND_INTERVAL) {
        const uint8_t type = type_[hc]; // Cache type lookup
        
        if (type == RC100H || type == RT800) {
            if (sendtype_[hc] == SendType::HUMI) { // send humidity
                if (switch_off_[hc]) {
                    remotehum_[hc] = EMS_VALUE_UINT8_NOTSET;
                }
                send_time_[hc] = uuid::get_uptime();
                humidity(addr, 0x10, hc);
                sendtype_[hc] = SendType::TEMP;
            } else { // temperature telegram
                if (remotehum_[hc] != EMS_VALUE_UINT8_NOTSET) {
                    sendtype_[hc] = SendType::HUMI;
                } else {
                    send_time_[hc] = uuid::get_uptime();
                }
                temperature(addr, 0x10, hc); // send to master-thermostat
            }
        } else if (type == RC200 || type == RC100 || type == FB10) {
            send_time_[hc] = uuid::get_uptime();
            temperature(addr, 0x10, hc); // send to master-thermostat (https://github.com/emsesp/EMS-ESP32/issues/336)
        } else { // type==RC20 or SENSOR
            send_time_[hc] = uuid::get_uptime();
            temperature(addr, 0x00, hc); // send to all
        }
        if (remotehum_[hc] == EMS_VALUE_UINT8_NOTSET && switch_off_[hc]) {
            switch_off_[hc] = false;
            type_[hc]       = RemoteType::NONE;
        }
    } else {
        // acknowledge every poll
        EMSuart::send_poll(addr | EMSbus::ems_mask());
    }
}

/**
 * check if there is a message for the remote room controller
 */
void Roomctrl::check(const uint8_t addr, const uint8_t * data, const uint8_t length) {
    // Early exit checks
    if (length < 5) {
        return;
    }
    
    const uint8_t hc = get_hc(addr);
    if (hc >= HCS) {
        return;
    }
    
    if (type_[hc] == SENSOR) {
        return;
    }
    
    // no reply if the temperature is not set
    if (remotetemp_[hc] == EMS_VALUE_INT16_NOTSET) {
        return;
    }
    
    // reply to writes with write acknowledgment
    if ((addr & 0x80) == 0) { // it's a write to us
        ack_write();              // accept writes, don't care.
        return;
    }
    
    const uint8_t masked_addr = addr & 0x7F;
    
    // reads: for now we only reply to version and remote temperature
    // empty message back if temperature not set or unknown message type
    const uint8_t msg_type = data[2];
    
    if (msg_type == EMSdevice::EMS_TYPE_VERSION) {
        version(masked_addr, data[0], hc);
    } else if (msg_type == 0xAF && data[3] == 0) {
        temperature(masked_addr, data[0], hc);
    } else if (length == 6) { // all other ems queries
        unknown(masked_addr, data[0], msg_type, data[3]);
    } else if (length == 8) {
        if (msg_type == 0xFF) {
            const uint8_t offset = data[3];
            const uint8_t typeh  = data[5];
            const uint8_t typel  = data[6];
            
            if (offset == 0) {
                if (typeh == 0 && typel == 0x23) { // Junkers
                    temperature(masked_addr, data[0], hc);
                } else if (typeh == 3) {
                    if (typel == 0x2B + hc) { // EMS+ temperature
                        temperature(masked_addr, data[0], hc);
                    } else if (typel == 0x7B + hc && remotehum_[hc] != EMS_VALUE_UINT8_NOTSET) { // EMS+ humidity
                        humidity(masked_addr, data[0], hc);
                    } else {
                        unknown(masked_addr, data[0], offset, typeh, typel);
                    }
                } else {
                    unknown(masked_addr, data[0], offset, typeh, typel);
                }
            } else {
                unknown(masked_addr, data[0], offset, typeh, typel);
            }
        } else if (msg_type == 0xF7) { // ems+ query with 3 bytes type src dst 7F offset len=FF FF HIGH LOW
            replyF7(masked_addr, data[0], data[3], data[5], data[6], data[7], hc);
        } else {
            unknown(masked_addr, data[0], data[3], data[5], data[6]);
        }
    }
}

/**
 * send version info
 */
void Roomctrl::version(const uint8_t addr, const uint8_t dst, const uint8_t hc) {
    uint8_t data[20];
    data[0] = addr | EMSbus::ems_mask();
    data[1] = dst & 0x7F;
    data[2] = 0x02;
    data[3] = 0;
    
    const uint8_t type = type_[hc]; // Cache type lookup
    data[4]            = type; // set RC20 id 113, Ver 02.01 or Junkers FB10 id 109, Ver 16.05, RC100H id 200 ver 40.04
    
    if (type == RC20) {
        data[5] = 2; // version 2.01
        data[6] = 1;
        data[7] = EMSbus::calculate_crc(data, 7); // append CRC
        EMSuart::transmit(data, 8);
    } else if (type == FB10) {
        data[5]  = 16; // version 16.05
        data[6]  = 5;
        data[7]  = 0;
        data[8]  = 0;
        data[9]  = 0;
        data[10] = 0;
        data[11] = 0;
        data[12] = 0;
        data[13] = 0;
        data[14] = EMSbus::calculate_crc(data, 14); // append CRC
        EMSuart::transmit(data, 15);
    } else if (type == RC200) {
        data[5]  = 32; // version 32.02 see #1611
        data[6]  = 2;
        data[7]  = 0;
        data[8]  = 0xFF;
        data[9]  = 0;
        data[10] = 0;
        data[11] = 0;
        data[12] = 0;
        data[13] = 0;
        data[14] = EMSbus::calculate_crc(data, 14); // append CRC
        EMSuart::transmit(data, 15);
    } else if (type == RC100H) {
        data[5] = 40; // version 40.04
        data[6] = 4;
        data[7] = 0;
        data[8] = 0xFF;
        data[9] = EMSbus::calculate_crc(data, 9); // append CRC
        EMSuart::transmit(data, 10);
    } else if (type == RC100) {
        data[5] = 40; // version 40.03
        data[6] = 3;
        data[7] = 0;
        data[8] = 0xFF;
        data[9] = EMSbus::calculate_crc(data, 9); // append CRC
        EMSuart::transmit(data, 10);
    } else if (type == RT800) {
        data[5] = 21; // version 21.03
        data[6] = 3;
        data[7] = EMSbus::calculate_crc(data, 7); // append CRC
        EMSuart::transmit(data, 8);
    }
}

/**
 * unknown message id, we reply with empty message
 */
void Roomctrl::unknown(const uint8_t addr, const uint8_t dst, const uint8_t type, const uint8_t offset) {
    uint8_t data[10];
    data[0] = addr | EMSbus::ems_mask();
    data[1] = dst & 0x7F;
    data[2] = type;
    data[3] = offset;
    data[4] = EMSbus::calculate_crc(data, 4); // append CRC
    EMSuart::transmit(data, 5);
}

void Roomctrl::unknown(const uint8_t addr, const uint8_t dst, const uint8_t offset, const uint8_t typeh, const uint8_t typel) {
    uint8_t data[10];
    data[0] = addr | EMSbus::ems_mask();
    data[1] = dst & 0x7F;
    data[2] = 0xFF;
    data[3] = offset;
    data[4] = typeh;
    data[5] = typel;
    data[6] = EMSbus::calculate_crc(data, 6); // append CRC
    EMSuart::transmit(data, 7);
}

/**
 * send the room temperature in message 0xAF
 */
void Roomctrl::temperature(const uint8_t addr, const uint8_t dst, const uint8_t hc) {
    uint8_t data[14];
    data[0] = addr | EMSbus::ems_mask();
    data[1] = dst & 0x7F;
    
    const uint8_t  type        = type_[hc]; // Cache type lookup
    const int16_t  remote_temp = remotetemp_[hc];
    const uint8_t  temp_high   = (uint8_t)(remote_temp >> 8);
    const uint8_t  temp_low    = (uint8_t)(remote_temp & 0xFF);
    
    if (type == RC20) { // RC20, telegram 0xAF
        data[2] = 0xAF;
        data[3] = 0;
        data[4] = temp_high;
        data[5] = temp_low;
        data[6] = 0;
        data[7] = EMSbus::calculate_crc(data, 7); // append CRC
        EMSuart::transmit(data, 8);
    } else if (type == FB10) { // Junkers FB10, telegram 0x0123
        data[2] = 0xFF;
        data[3] = 0;
        data[4] = 0;
        data[5] = 0x23; //  fixed for all hc
        data[6] = temp_high;
        data[7] = temp_low;
        data[8] = EMSbus::calculate_crc(data, 8); // append CRC
        EMSuart::transmit(data, 9);
    } else if (type == RC200) { // RC200, telegram 42B, ff
        data[2]        = 0xFF;
        data[3]        = 0;
        data[4]        = 3;
        data[5]        = 0x2B + hc;
        data[6]        = temp_high;
        data[7]        = temp_low;
        const uint16_t t1 = remote_temp * 10 + 3;
        data[8]        = (uint8_t)(t1 >> 8);
        data[9]        = (uint8_t)(t1 & 0xFF);
        data[10]       = 1;                               // not sure what this is and if we need it, maybe mode?
        data[11]       = EMSbus::calculate_crc(data, 11); // append CRC
        EMSuart::transmit(data, 12);
    } else if (type == RC100H || type == RC100) { // RC100H, telegram 42B, ff
        data[2] = 0xFF;
        data[3] = 0;
        data[4] = 3;
        data[5] = 0x2B + hc;
        data[6] = temp_high;
        data[7] = temp_low;
        data[8] = EMSbus::calculate_crc(data, 8); // append CRC
        EMSuart::transmit(data, 9);
    } else if (type == SENSOR) { // wireless sensor, broadcast id 435
        data[2] = 0xFF;
        data[3] = 0;
        data[4] = 3;
        data[5] = 0x35 + hc;
        data[6] = temp_high;
        data[7] = temp_low;
        data[8] = EMSbus::calculate_crc(data, 8); // append CRC
        EMSuart::transmit(data, 9);
    } else if (type == RT800) { // RT800, telegram 42B, ff
        data[2]        = 0xFF;
        data[3]        = 0;
        data[4]        = 3;
        data[5]        = 0x2B + hc;
        data[6]        = temp_high;
        data[7]        = temp_low;
        const uint16_t t1 = remote_temp * 10 + 3;
        data[8]        = (uint8_t)(t1 >> 8);
        data[9]        = (uint8_t)(t1 & 0xFF);
        data[10]       = 1;                               // not sure what this is and if we need it, maybe mode?
        data[11]       = 9;                               // not sure what this is and if we need it, maybe mode?
        data[12]       = EMSbus::calculate_crc(data, 12); // append CRC
        EMSuart::transmit(data, 13);
    }
}

// send telegram 0x047B only for RC100H
void Roomctrl::humidity(const uint8_t addr, const uint8_t dst, const uint8_t hc) {
    uint8_t        data[11];
    data[0]           = addr | EMSbus::ems_mask();
    data[1]           = dst & 0x7F;
    const uint16_t dew = calc_dew(remotetemp_[hc], remotehum_[hc]);
    data[2]           = 0xFF;
    data[3]           = 0;
    data[4]           = 3;
    data[5]           = 0x7B + hc;
    data[6]           = (dew == EMS_VALUE_INT16_NOTSET) ? EMS_VALUE_INT8_NOTSET : (uint8_t)((dew + 5) / 10);
    data[7]           = remotehum_[hc];
    data[8]           = (uint8_t)(dew >> 8);
    data[9]           = (uint8_t)(dew & 0xFF);
    data[10]          = EMSbus::calculate_crc(data, 10); // append CRC
    EMSuart::transmit(data, 11);
}

/**
 * send a nack if someone want to write to us.
 */
void Roomctrl::nack_write() {
    uint8_t data[1];
    data[0] = TxService::TX_WRITE_FAIL;
    EMSuart::transmit(data, 1);
}

/**
 * send a ack if someone want to write to us.
 */
void Roomctrl::ack_write() {
    uint8_t data[1];
    data[0] = TxService::TX_WRITE_SUCCESS;
    EMSuart::transmit(data, 1);
}

void Roomctrl::replyF7(const uint8_t addr, const uint8_t dst, const uint8_t offset, const uint8_t typehh, const uint8_t typeh, const uint8_t typel, const uint8_t hc) {
    uint8_t data[12];
    data[0] = addr | EMSbus::ems_mask();
    data[1] = dst & 0x7F;
    data[2] = 0xF7;
    data[3] = offset;
    data[4] = typehh;
    data[5] = typeh;
    data[6] = typel;
    if (typehh == 0x02) {
        if (type_[hc] == RC200 || type_[hc] == FB10) {
            data[7] = 0xFF;
            data[8] = 0x01;
        } else {
            data[7] = 0x0F;
            data[8] = 0x00;
        }
    } else {
        data[7] = 0;
        data[8] = 0;
    }
    data[9] = EMSbus::calculate_crc(data, 9); // append CRC
    EMSuart::transmit(data, 10);
}

int16_t Roomctrl::calc_dew(const int16_t temp, const uint8_t humi) {
    if (humi == EMS_VALUE_UINT8_NOTSET || temp == EMS_VALUE_INT16_NOTSET) {
        return EMS_VALUE_INT16_NOTSET;
    }
    
    constexpr float k2 = 17.62;
    constexpr float k3 = 243.12;
    
    const float t  = static_cast<float>(temp) / 10.0f;
    const float h  = static_cast<float>(humi) / 100.0f;
    const float ln = log(h);
    const float k2t_k3t = (k2 * t) / (k3 + t);
    
    const int16_t dt = static_cast<int16_t>((10 * k3 * (k2t_k3t + ln)) / (((k2 * k3) / (k3 + t)) - ln));
    return dt;
}


} // namespace emsesp

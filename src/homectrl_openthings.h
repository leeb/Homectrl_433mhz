/**
 * This file is part of the Homectrl 433mhz distribution (https://github.com/leeb/Homectrl_433mhz)
 * Copyright (c) 2020 Lee Briggs
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _HOMECTRL_OPENTHINGS_H_
#define _HOMECTRL_OPENTHINGS_H_

namespace homectrl {

#define OPENTHINGS_ALARM            0x21
#define OPENTHINGS_DOOR_BELL        0x44
#define OPENTHINGS_SMOKE_DETECTOR   0x53
#define OPENTHINGS_FREQUENCY        0x66
#define OPENTHINGS_JOIN             0x6A
#define OPENTHINGS_LIGHT_LEVEL      0x6C
#define OPENTHINGS_MOTION_DETECTOR  0x6D
#define OPENTHINGS_OCCUPANCY        0x6F
#define OPENTHINGS_REAL_POWER       0x70
#define OPENTHINGS_REACTIVE_POWER   0x71
#define OPENTHINGS_SWITCH_STATE     0x73
#define OPENTHINGS_TEMPERATURE      0x74
#define OPENTHINGS_VOLTAGE          0x76

#define OPENTHINGS_HEADER_SIZE      8

#define RF_FRF_OPENTHINGS           0x6C9333    // 434.3mhz


typedef struct OpenThingsHeader {
  uint8_t   size;
  uint8_t   manufacturer_id;
  uint8_t   product_id;
  uint16_t  reserved;
  uint32_t  sensor_id;  
} OpenThingsHeader;

void openthings_crypt(uint8_t *buf, uint8_t len, uint16_t seed);
uint16_t openthings_crc(uint8_t *buf, uint8_t len);


} // namespace homectrl

#endif

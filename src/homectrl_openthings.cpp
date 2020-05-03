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

#include <Arduino.h>
#include "homectrl_openthings.h"

namespace homectrl {

void openthings_crypt(uint8_t *buf, uint8_t len, uint16_t seed) {
  uint8_t i, r;
  
  seed ^= (242 << 8);

  for (i = 5; i < len; i++) {
    for (r = 0; r < 5; r++) {
      seed = (seed & 1) ? ((seed >> 1) ^ 62965U) : (seed >> 1);
    }      
    buf[i] = (uint8_t)(seed ^ buf[i] ^ 90U);
  }  
}

uint16_t openthings_crc(uint8_t *buf, uint8_t len) {
  uint8_t i, r;
  uint16_t crc = 0;

  for (i = 5; i < len; i++) {
    crc ^= (buf[i] << 8);
    for (r = 8; r > 0; r--) {
      crc = ((crc & (1 << 15)) ? ((crc << 1) ^ 0x1021) : (crc << 1)); 
    }    
  }
  return crc;
}

} // namespace homectrl

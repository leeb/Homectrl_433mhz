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
#ifndef _HOMECTRL_ENERGENIE_H_
#define _HOMECTRL_ENERGENIE_H_

#include <functional>

#include "homectrl_rfm69.h"

namespace homectrl {

// SDR tuning results
//   Gateway on 433.92
//   Remote on 433.98
// Frequency calculator : hz * << 11 / 125
//   433.92 = 0x6C7AE1
//   433.98 = 0x6C7EB8
#define RF_FRF_ENERGENIE           0x6C7AE1    // 433.92mhz
//#define RF_FRF_ENERGENIE           0x6C7E00    


typedef struct EnergenieMsg {
  uint32_t  address;    // 20 bits for address
  uint8_t   state;      // 4 bits for switch state
} EnergenieMsg;


/**
 * Interface for Legacy Energenie devices which receive commands using OOK modulation.
 * Supported:
 *   - MIHO002 Smart Plug.
 */
class Energenie {
 private:
  Rfm69*    rfm69;
  uint8_t   tx_repeats;

 public:
  Energenie();
  void begin(Rfm69&);
 
  void modeTransmit();
  void modeReceive();

  void sendState(uint8_t state, uint32_t addr);
  uint8_t receivePayload();
  void transmitPayload(uint8_t *buf);

  void onEnergenieMsg(std::function<void(EnergenieMsg*)> cb) { cb_energenie_msg = cb; };

 private:
  std::function<void(EnergenieMsg*)> cb_energenie_msg;
};
  
} // namespace homectrl

#endif

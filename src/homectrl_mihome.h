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
#ifndef _HOMECTRL_MIHOME_H_
#define _HOMECTRL_MIHOME_H_

#include <functional>

#include "homectrl_rfm69.h"
#include "homectrl_openthings.h"


namespace homectrl {

#define MANUFACTURER_MIHOME         0x04

#define MIHOME_MONITOR              0x01
#define MIHOME_ADAPTER_PLUS         0x02

// Status message from MIHO004 Smart Monitor Plug
typedef struct MiHomeMonitorMsg : OpenThingsHeader {
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  bool      join;  
} MiHomeMonitorMsg;

// Status message from MIHO005 Smart Plug Plus
typedef struct MiHomeAdapterPlusMsg : OpenThingsHeader {
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  uint8_t   switch_state;
  bool      join;  
} MiHomeAdapterPlusMsg;

// Update message from MIHO005 Smart Plug Plus when switch state changes
typedef struct MiHomeSwitchStateMsg : OpenThingsHeader {
  uint8_t   switch_state;
} MiHomeSwitchStateMsg;

/**
 * Interface for Energenie MiHome devices based on the OpenThings protocol.
 * These use FSK modulation and can both transmit and receive data.
 * Supported:
 *   - MIHO004 Smart Monitor Plug
 *   - MIHO005 Smart Plug Plus
 */
class MiHome {
 public:
  MiHome();
  void begin(Rfm69&);
  void setDebugLevel(uint8_t);
  void modeTransmit();
  void modeReceive();
  uint8_t receivePayload();

  void transmitPayload(uint8_t *buf, uint8_t len);
  void sendSwitchState(uint8_t switch_state, uint32_t sensor_id, uint8_t product_id=MIHOME_ADAPTER_PLUS, uint8_t manufacturer=MANUFACTURER_MIHOME);

  void dumpHeader(OpenThingsHeader*);
  void dumpMessage(MiHomeAdapterPlusMsg*);
  void dumpMessage(MiHomeSwitchStateMsg*);
  void dumpMessage(MiHomeMonitorMsg*);
  
  void onMsg(std::function<void(OpenThingsHeader*)> cb) { cb_openthings_msg = cb; };
  void onSwitchStateMsg(std::function<void(MiHomeSwitchStateMsg*)> cb) { cb_switch_state_msg = cb; };
  void onAdapterPlusMsg(std::function<void(MiHomeAdapterPlusMsg*)> cb) { cb_adapter_plus_msg = cb; };
  void onMonitorMsg(std::function<void(MiHomeMonitorMsg*)> cb) { cb_monitor_msg = cb; };

 private:
  Rfm69     *rfm69;

  void receiveHeader(uint8_t*, OpenThingsHeader*);
  void receiveSwitchState(uint8_t*, uint8_t);
  void receiveAdapterPlus(uint8_t*, uint8_t);
  void receiveMonitor(uint8_t*, uint8_t);

  void unassigned() { return; };

  std::function<void(OpenThingsHeader*)> cb_openthings_msg;
  std::function<void(MiHomeSwitchStateMsg*)> cb_switch_state_msg;
  std::function<void(MiHomeAdapterPlusMsg*)> cb_adapter_plus_msg;
  std::function<void(MiHomeMonitorMsg*)> cb_monitor_msg = {};
};

} // namespace homectrl

#endif

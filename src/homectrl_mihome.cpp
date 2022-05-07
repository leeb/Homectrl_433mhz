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
#include <RFM69registers.h>
// #include <functional>

#include "homectrl_rfm69.h"
#include "homectrl_openthings.h"
#include "homectrl_mihome.h"

namespace homectrl {

// #define _MIHOME_DEBUG_

MiHome::MiHome() {

  onMsg([](OpenThingsHeader *msg) {
    #ifdef _MIHOME_DEBUG_
    Serial.println("onMsg callback unassigned");
    #endif
  });
  onSwitchStateMsg([](MiHomeSwitchStateMsg *msg) {
    #ifdef _MIHOME_DEBUG_
    Serial.println("onSwitchStateMsg callback unassigned");
    #endif
  });
  onAdapterPlusMsg([](MiHomeAdapterPlusMsg *msg) {
    #ifdef _MIHOME_DEBUG_
    Serial.println("onAdapterPlusMsg callback unassigned");
    #endif
  });
  onMonitorMsg([](MiHomeMonitorMsg *msg) {
    #ifdef _MIHOME_DEBUG_
    Serial.println("onMonitorMsg callback unassigned");
    #endif
  });
}


void MiHome::begin(Rfm69& rfm69) {
  this->rfm69 = &rfm69;
}


void MiHome::modeTransmit() {
  rfm69->setModeStandby();

  rfm69->setModulation(RF_DATAMODUL_MODULATIONTYPE_FSK);
  rfm69->setFrequencyDeviation(RF_FDEV_5000);
  rfm69->setFrequency(RF_FRF_OPENTHINGS);
  rfm69->setPower(RF_PALEVEL_PA1_ON | RF_PALEVEL_OUTPUTPOWER_11111);

  rfm69->setBitrate(RF_BITRATE_4800);
  rfm69->setPreamble(3);
  rfm69->setSync(RF_SYNC_ON | RF_SYNC_SIZE_2, 0x2DD40000);

  rfm69->setPacketConfig(RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_MANCHESTER, RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
  rfm69->setFifoThreshold(RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY);

  // switch to trasnmit if fifo not empty, back to standby once complete
  rfm69->setAutomode(RF_AUTOMODES_ENTER_FIFONOTEMPTY | RF_AUTOMODES_EXIT_PACKETSENT | RF_AUTOMODES_INTERMEDIATE_TRANSMITTER);

  rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void MiHome::dumpHeader(OpenThingsHeader *hdr) {
  #ifdef _MIHOME_DEBUG_
  char str[24];
  
  sprintf(str, "HDR: manufacturer: %02X", hdr->manufacturer_id);
  Serial.print(str);
  sprintf(str, "  product: %02X", hdr->product_id);
  Serial.print(str);
  sprintf(str, "  reserved: %04X", hdr->reserved);
  Serial.print(str);  
  sprintf(str, "  sensor: %06X", hdr->sensor_id);
  Serial.println(str);
  #endif
}


void MiHome::dumpMessage(MiHomeAdapterPlusMsg *msg) {
  #ifdef _MIHOME_DEBUG_
  Serial.println("AdapterPlus message");
  dumpHeader(msg);
  Serial.print("Voltage: ");
  Serial.println(msg->voltage);
  Serial.print("Reactive power: ");
  Serial.println(msg->reactive_power);
  Serial.print("Real Power: ");
  Serial.println(msg->real_power);
  Serial.print("Frequency: ");
  Serial.println((float)msg->frequency / 256);
  Serial.print("Switch state: ");
  Serial.println(msg->switch_state);
  #endif
}


void MiHome::dumpMessage(MiHomeSwitchStateMsg *msg) {
  #ifdef _MIHOME_DEBUG_
  Serial.println("SwitchState message");
  dumpHeader(msg);  
  Serial.print("Switch state: ");
  Serial.println(msg->switch_state);
  #endif
}


void MiHome::dumpMessage(MiHomeMonitorMsg *msg) {
  #ifdef _MIHOME_DEBUG_
  Serial.println("Monitor message");
  dumpHeader(msg);
  Serial.print("Voltage: ");
  Serial.println(msg->voltage);
  Serial.print("Reactive power: ");
  Serial.println(msg->reactive_power);
  Serial.print("Real Power: ");
  Serial.println(msg->real_power);
  Serial.print("Frequency: ");
  Serial.println((float)msg->frequency / 256);
  #endif
}


void MiHome::modeReceive() {
  rfm69->setModeRx();

  rfm69->setModulation(RF_DATAMODUL_MODULATIONTYPE_FSK);
  rfm69->setFrequencyDeviation(RF_FDEV_5000);
  rfm69->setFrequency(RF_FRF_OPENTHINGS);
  rfm69->setBandwidth(RF_RXBW_60);

  //rfm69->writeReg(REG_AFCCTRL, 0x00);
  //rfm69->writeReg(REG_LNA, RF_LNA_ZIN_50);

  rfm69->setBitrate(RF_BITRATE_4800);
  rfm69->setPreamble(3);
  rfm69->setSync(RF_SYNC_ON | RF_SYNC_SIZE_2, 0x2DD40000);

  rfm69->setPacketConfig(RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_MANCHESTER, RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
  rfm69->setPayloadLength(RF_PAYLOADLENGTH_VALUE);
  rfm69->setAutomode(RF_AUTOMODES_ENTER_OFF | RF_AUTOMODES_EXIT_OFF);
}


void MiHome::receiveHeader(uint8_t *buf, OpenThingsHeader *msg) {
  msg->manufacturer_id = buf[1];
  msg->product_id = buf[2];  
  msg->reserved = buf[3] << 8 | buf[4];
  msg->sensor_id = buf[5] << 16 | buf[6] << 8 | buf[7];  
}


void MiHome::receiveSwitchState(uint8_t *buf, uint8_t len) {
  MiHomeSwitchStateMsg msg = {};
  uint8_t i = OPENTHINGS_HEADER_SIZE;

  receiveHeader(buf, &msg);
  if (buf[i] == OPENTHINGS_SWITCH_STATE && buf[i + 1] == 0x01) {
    msg.switch_state = buf[i + 2];
  }
  cb_switch_state_msg(&msg);  
}


void MiHome::receiveAdapterPlus(uint8_t *buf, uint8_t len) {
  MiHomeAdapterPlusMsg msg = {};
  uint8_t param, d_type, d_size, i = OPENTHINGS_HEADER_SIZE;

  receiveHeader(buf, &msg);

  while (i < len) {
    param = buf[i];
    i++;

    if (param == 0x0) break;

    d_type = buf[i] >> 4;
    d_size = buf[i] & 0x0f;
    i++;

    switch (param) {
      case OPENTHINGS_REAL_POWER:
        if (d_type == 8 && d_size == 2)
           msg.real_power = buf[i] << 8 | buf[i + 1];
        break;

      case OPENTHINGS_REACTIVE_POWER:
        if (d_type == 8 && d_size == 2)
          msg.reactive_power = buf[i] << 8 | buf[i + 1];
        break;

      case OPENTHINGS_VOLTAGE:
        if (d_type == 0 && d_size == 1)
          msg.voltage = buf[i];
        break;

      case OPENTHINGS_FREQUENCY:
        if (d_type == 2 && d_size == 2)
          msg.frequency = buf[i] << 8 | buf[i + 1];
        break;

      case OPENTHINGS_SWITCH_STATE:
        msg.switch_state = buf[i];
        break;
    }
    i += d_size;
  }
  cb_adapter_plus_msg(&msg);
}


void MiHome::receiveMonitor(uint8_t *buf, uint8_t len) {
  MiHomeMonitorMsg msg = {};
  uint8_t param, d_type, d_size, i = OPENTHINGS_HEADER_SIZE;

  receiveHeader(buf, &msg);

  while (i < len) {
    param = buf[i];
    i++;

    if (param == 0x0) break;

    d_type = buf[i] >> 4;
    d_size = buf[i] & 0x0f;
    i++;

    switch (param) {
      case OPENTHINGS_REAL_POWER:
        if (d_type == 8 && d_size == 2)
           msg.real_power = buf[i] << 8 | buf[i + 1];
        break;

      case OPENTHINGS_REACTIVE_POWER:
        if (d_type == 8 && d_size == 2)
          msg.reactive_power = buf[i] << 8 | buf[i + 1];
        break;

      case OPENTHINGS_VOLTAGE:
        if (d_type == 0 && d_size == 1)
          msg.voltage = buf[i];
        break;

      case OPENTHINGS_FREQUENCY:
        if (d_type == 2 && d_size == 2)
          msg.frequency = buf[i] << 8 | buf[i + 1];
        break;

    }
    i += d_size;
  }
  cb_monitor_msg(&msg);
}


uint8_t MiHome::receivePayload() {
  uint8_t  buf[66];
  uint8_t  len, i;
  uint16_t seed, crc;
  uint32_t sensor_id;

  len = rfm69->readFifo(buf);

  // Serial.print("Buffer received ");
  // Serial.println(len);

  if (len < 8) return 0;
  if (len != buf[0] + 1) return 0;  // check header size matches size
  if (buf[1] != MANUFACTURER_MIHOME) return 0;     // manufacturer must be mihome
  // if (buf[2] != 0x02 
  seed = buf[3] << 8 | buf[4];

  openthings_crypt(buf, len, seed);
  sensor_id = buf[5] << 16 | buf[6] << 8 | buf[7];
  crc = (buf[len - 2] << 8) | buf[len - 1];

  if (crc != openthings_crc(buf, len - 2)) {
    #ifdef _MIHOME_DEBUG_
    Serial.println("bad crc");
    #endif
    return 0;
  }

  switch (buf[2]) {
    case MIHOME_ADAPTER_PLUS: {
      if (len == 14) {
        receiveSwitchState(buf, len); 
      } else {
        receiveAdapterPlus(buf, len);
      }
      break;
    }
    case MIHOME_MONITOR: {
      receiveMonitor(buf, len);
      break;
    }
    default:
      Serial.println("Unknown device");
  }

  #ifdef _MIHOME_DEBUG_
  /*
  for (i = 0; i < len; i++) {
    char str[5];
    sprintf(str, "%02X ", buf[i]);
    Serial.print(str);
  }   
  Serial.println("");
  Serial.println("");
  */
  #endif

  return len;
}


void MiHome::transmitPayload(uint8_t *buf, uint8_t len) {
  if (rfm69->waitFor(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFONOTEMPTY, false)) {
    rfm69->writeFifo(buf, len);
    rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_AUTOMODE, false);    // wait for automode to clear, so transmission finished.
  }
}


void MiHome::sendSwitchState(uint8_t switch_state, uint32_t sensor_id, uint8_t product_id, uint8_t manufacturer_id) {
  MiHomeSwitchStateMsg msg;
  uint8_t buf[66];
  uint8_t len;
  uint16_t crc;

  msg.manufacturer_id = manufacturer_id;
  msg.product_id = product_id;
  msg.sensor_id = sensor_id;
  msg.reserved = (uint16_t)millis();
  msg.switch_state = switch_state ? 0 : 1;

  buf[1] = msg.manufacturer_id;
  buf[2] = msg.product_id;
  buf[3] = msg.reserved >> 8 & 0xff;
  buf[4] = msg.reserved & 0xff;
  buf[5] = msg.sensor_id >> 16 & 0xff;
  buf[6] = msg.sensor_id >> 8 & 0xff;
  buf[7] = msg.sensor_id & 0xff;
  len = OPENTHINGS_HEADER_SIZE;

  buf[8] = 0x80 | OPENTHINGS_SWITCH_STATE;
  buf[9] = 1;   // datatype | length
  buf[10] = (msg.switch_state) ? 0 : 1;
  buf[11] = 0x0;    // terminate
  len += 4;

  crc = openthings_crc(buf, len);
  buf[12] = crc >> 8 & 0xff;
  buf[13] = crc & 0xff;
  len +=2 ;
  buf[0] = len - 1;
  
  openthings_crypt(buf, len, msg.reserved);
  transmitPayload(buf, len);
}

} // namespace homectrl

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

#include "homectrl_rfm69.h"
#include "homectrl_energenie.h"

namespace homectrl {

#define _ENERGENIE_DEBUG_

//  1,000,000 / 225ms = 444mhz
// 32,000,000 / 4444 = 7200
// 215ms = 6880
// 210ms = 6720
// 4800bps = 6667
#define RF_BITRATE RF_BITRATE_4800

/* 
  SDR tuning results
    Gateway on 433.92
    Remote on 433.98
  Frequency calculator : hz * << 11 / 125
    433.92 = 0x6C7AE1
    433.98 = 0x6C7EB8
*/
#define RF_FRF_ENERGENIE           0x6C7AE1    // 433.92mhz
//#define RF_FRF_ENERGENIE           0x6C7E80   // for remote ctrl test


Energenie::Energenie() {
  onEnergenieMsg([](EnergenieMsg *msg) {
    #ifdef _ENERGENIE_DEBUG_
    Serial.println("onEnergenieMsg callback unassigned");
    #endif
  });
  tx_repeats = 8;
}


void Energenie::begin(Rfm69& rfm69) {
  this->rfm69 = &rfm69;
}


void Energenie::modeTransmit() {
  rfm69->setModeStandby();

  rfm69->setModulation(RF_DATAMODUL_MODULATIONTYPE_OOK);
  rfm69->setFrequencyDeviation(RF_FDEV_5000);
  rfm69->setFrequency(RF_FRF_ENERGENIE);

  rfm69->setPower(RF_PALEVEL_PA1_ON | RF_PALEVEL_OUTPUTPOWER_11111);

  rfm69->setBitrate(RF_BITRATE);
  rfm69->setPreamble(0);
  rfm69->setSync(RF_SYNC_OFF, 0);

  //rfm69->setPacketConfig(RF_PACKET1_FORMAT_VARIABLE, RF_PACKET2_AES_OFF);
  //rfm69->setPayloadLength(0x0);  // variable packt lenght
  //rfm69->setFifoThreshold(RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY);

  rfm69->setPacketConfig(RF_PACKET1_FORMAT_FIXED, RF_PACKET2_AES_OFF);
  rfm69->setPayloadLength(16);  // variable packt length
  rfm69->setFifoThreshold(RF_FIFOTHRESH_TXSTART_FIFOTHRESH | 15);

  // switch to trasnmit when threshold reached, back to standby once complete
  rfm69->setAutomode(RF_AUTOMODES_ENTER_FIFOLEVEL | RF_AUTOMODES_EXIT_PACKETSENT | RF_AUTOMODES_INTERMEDIATE_TRANSMITTER);

  rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void Energenie::modeReceive() {
  rfm69->setModeRx();

  rfm69->setModulation(RF_DATAMODUL_MODULATIONTYPE_OOK);
  rfm69->setFrequencyDeviation(RF_FDEV_5000);
  rfm69->setFrequency(RF_FRF_ENERGENIE);
  rfm69->setBandwidth(RF_RXBW_120);

  rfm69->setBitrate(RF_BITRATE);
  rfm69->setPreamble(0);
  rfm69->setSync(RF_SYNC_ON | RF_SYNC_SIZE_4, 0x80000000);

  rfm69->setPacketConfig(RF_PACKET1_FORMAT_FIXED, RF_PACKET2_AES_OFF);
  rfm69->setPayloadLength(12);  // fixed packet length

  rfm69->setAutomode(RF_AUTOMODES_ENTER_OFF | RF_AUTOMODES_EXIT_OFF);

  rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void Energenie::sendState(uint8_t state, uint32_t addr) {
  uint8_t  buf[12];
  uint32_t v = (addr << 4) | (state & 0xf);

  for (uint8_t i = 0; i < 12; i++) {
    v <<= 2;

    switch (v & 0x3000000) {

      case 0x0000000:
        buf[i] = 0xee; // ---_---_
        //buf[i] = 0x77; // ---_---_
        break;      
      case 0x1000000:
        buf[i] = 0xe8; // ---_-___
        //buf[i] = 0x74; // ---_-___
        break;
      case 0x2000000:
        buf[i] = 0x8e; // -___---_
        //buf[i] = 0x47; // -___---_
        break;
      case 0x3000000:
        buf[i] = 0x88; // -___-___
        //buf[i] = 0x44; // -___-___
        break;
    }
  }
  transmitPayload(buf);
}


uint8_t Energenie::receivePayload() {
  uint8_t  buf[66];
  uint8_t  len, i, b;
  uint32_t addr;

  len = rfm69->readFifo(buf);

  if (len == 12) {
    /*
    for (i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    */
    // The energie remote has a slightly longer delay,
    // if the pattern matches this shift them forward 1 bit.

    b = 0x77;
    for (i = 0; i < 12; i++) b |= buf[1];
    if (b == 0x77) {
      for (i = 0; i < 12; i++) buf[i] <<= 1;
    }

    // appears to be a 24bit energenie packet
    addr = 0x0;

    for (i = 0; i < 12; i++) {
      addr <<= 2;
      switch (buf[i]) {
        case 0xee:
          addr |= 0;
          break;
        case 0xe8:
          addr |= 1;
          break;
        case 0x8e:
          addr |= 2;
          break;
        case 0x88:
          addr |= 3;
          break;
        default:
          // corrupt code, exit
          return 0;   
      }
    }
    // good packet decode.

    EnergenieMsg msg;

    msg.address = addr >> 4;
    msg.state = addr & 0x00000f;
    cb_energenie_msg(&msg);
  }
  return len;
}


void Energenie::transmitPayload(uint8_t *buf) {
  uint8_t hdr[8] = {0x80, 0, 0, 0};
  uint8_t r;
  
  // wait for fifo to be empty
  if (rfm69->waitFor(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFONOTEMPTY, false)) {  
    for (r = 0; r < tx_repeats; r++) {  
      // wait for automode to clear, so transmission finished.
      if (rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_AUTOMODE, false, 50)) {
        // header after data matches output from remote
        rfm69->writeFifo(buf, 12);
        rfm69->writeFifo(hdr, 4);
      }
    }
    rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_AUTOMODE, false);    // wait for automode to clear, so transmission finished.
  }
}

} // namespace homectrl

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


Energenie::Energenie() {
  tx_repeats = 15;
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

  rfm69->setBitrate(RF_BITRATE_4800);
  rfm69->setPreamble(0);
  rfm69->setSync(RF_SYNC_OFF, 0x00000000);
  // #rfm69_set_sync(RF_SYNC_ON | RF_SYNC_SIZE_4, [0x80,0x00,0x00,0x00])

  rfm69->setPacketConfig(RF_PACKET1_FORMAT_FIXED, RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
  rfm69->setPayloadLength(64);
  rfm69->setFifoThreshold(RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | 15);
  rfm69->setAutomode(RF_AUTOMODES_ENTER_FIFONOTEMPTY | RF_AUTOMODES_EXIT_PACKETSENT | RF_AUTOMODES_INTERMEDIATE_TRANSMITTER);

  rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void Energenie::modeReceive() {
  rfm69->setModeRx();

  rfm69->setModulation(RF_DATAMODUL_MODULATIONTYPE_OOK);
  rfm69->setSync(RF_SYNC_ON, 0x80000000);

  rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void Energenie::sendState(uint8_t state, uint32_t addr) {
  uint8_t  buf[12];
  uint32_t v = (addr << 4) | (state & 0xf);

  for (uint8_t i = 0; i < 12; i++) {
    v <<= 2;
    switch (v & 0x3000000) {
      case 0x0000000:
        buf[i] = 0x77;
        break;      
      case 0x1000000:
        buf[i] = 0x74;
        break;
      case 0x2000000:
        buf[i] = 0x47;
        break;
      case 0x3000000:
        buf[i] = 0x44;
        break;
    }
  }
  transmitPayload(buf);
}


void Energenie::transmitPayload(uint8_t *buf) {
  uint8_t hdr[4] = {0x80, 0, 0, 0};
  uint8_t r;
  
  if (rfm69->waitFor(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFONOTEMPTY, false)) {

    for (r = 0; r < tx_repeats; r++) {
      rfm69->waitFor(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOLEVEL, false);
      rfm69->writeFifo(hdr, 4);
      rfm69->writeFifo(buf, 12);
    }
    rfm69->waitFor(REG_IRQFLAGS1, RF_IRQFLAGS1_AUTOMODE, false);    // wait for automode to clear, so transmission finished.
  }
}

} // namespace homectrl

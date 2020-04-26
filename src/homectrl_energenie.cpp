#include <Arduino.h>
#include <RFM69registers.h>

#include "homectrl_rfm69.h"
#include "homectrl_energenie.h"

namespace homectrl {


Energenie::Energenie() {
  tx_repeats = 15;
}


void Energenie::modeTransmit() {
  rfm69_set_mode_standby();

  rfm69_set_modulation(RF_DATAMODUL_MODULATIONTYPE_OOK);
  rfm69_set_frequency_deviation(RF_FDEV_5000);
  rfm69_set_frequency(RF_FRF_ENERGENIE);
  rfm69_set_power(RF_PALEVEL_PA1_ON | RF_PALEVEL_OUTPUTPOWER_11111);

  rfm69_set_bitrate(RF_BITRATE_4800);
  rfm69_set_preamble(0);
  rfm69_set_sync(RF_SYNC_OFF, 0x00000000);
  // #rfm69_set_sync(RF_SYNC_ON | RF_SYNC_SIZE_4, [0x80,0x00,0x00,0x00])

  rfm69_set_packet_config(RF_PACKET1_FORMAT_FIXED, RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
  rfm69_set_payload_length(64);
  rfm69_set_fifo_threshold(RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | 15);
  rfm69_set_automode(RF_AUTOMODES_ENTER_FIFONOTEMPTY | RF_AUTOMODES_EXIT_PACKETSENT | RF_AUTOMODES_INTERMEDIATE_TRANSMITTER);

  rfm69_wait_for(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
}


void Energenie::modeReceive() {
  rfm69_set_mode_rx();

  rfm69_set_modulation(RF_DATAMODUL_MODULATIONTYPE_OOK);
  rfm69_set_sync(RF_SYNC_ON, 0x80000000);

  rfm69_wait_for(REG_IRQFLAGS1, RF_IRQFLAGS1_MODEREADY, true);
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
  
  if (rfm69_wait_for(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFONOTEMPTY, false)) {

    for (r = 0; r < tx_repeats; r++) {
      rfm69_wait_for(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOLEVEL, false);
      rfm69_write_fifo(hdr, 4);
      rfm69_write_fifo(buf, 12);
    }
    rfm69_wait_for(REG_IRQFLAGS1, RF_IRQFLAGS1_AUTOMODE, false);    // wait for automode to clear, so transmission finished.
  }
}


} // namespace homectrl

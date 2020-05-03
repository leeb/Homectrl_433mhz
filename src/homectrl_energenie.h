#ifndef _HOMECTRL_ENERGENIE_H_
#define _HOMECTRL_ENERGENIE_H_

#include "homectrl_rfm69.h"

namespace homectrl {

#define RF_FRF_ENERGENIE           0x6C7AE1    // 433.92mhz

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
  void transmitPayload(uint8_t *buf);
};
  
} // namespace homectrl

#endif

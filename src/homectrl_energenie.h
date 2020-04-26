#ifndef _HOMECTRL_ENERGENIE_H_
#define _HOMECTRL_ENERGENIE_H_

namespace homectrl {

#define RF_FRF_ENERGENIE           0x6C7AE1    // 433.92mhz

class Energenie {
 public:
  Energenie();
 
  void modeTransmit();
  void modeReceive();

  void sendState(uint8_t state, uint32_t addr);
  void transmitPayload(uint8_t *buf);
 
 private:
  uint8_t tx_repeats;
};
  
} // namespace homectrl

#endif

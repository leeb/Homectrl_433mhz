/**
 *
 * TODO hc/hcw switch
 * 
 */

#ifndef _HOMECTRL_RFM69_H_
#define _HOMECTRL_RFM69_H_

#include <SPI.h>

#include "homectrl_rfm69_registers.h"

#define RF_FDEV_5000       0x0052

#define RF_BITRATE_1200    0x682B
#define RF_BITRATE_2400    0x3415
#define RF_BITRATE_4800    0x1A0B
#define RF_BITRATE_9600    0x0D05
#define RF_BITRATE_19200   0x0683
#define RF_BITRATE_38400   0x0341
#define RF_BITRATE_57600   0x022C


#if defined ARDUINO_ARCH_ESP32

#define RFM69_SPI_BUS     VSPI
#define RFM69_SS_PIN      SS    // GPIO 5
#define RFM69_RST_PIN     4     // GPIO 4 
#define RFM69_DIO0_PIN    15    // GPIO 15

#else   // default assumes __AVR32__

#define RFM69_SS_PIN      10    //
#define RFM69_RST_PIN     9     //
#define RFM69_D1O0_PIN    8     //

#endif

namespace homectrl {

class Rfm69 {
 public:
  enum Hardware { RFM69W = 1, RFM69HW };

 private:
  Hardware        _hardware;
  bool            _highPower;
  uint8_t         _pa;
  SPIClass*       _spi;
  SPISettings     _spi_settings;
  uint8_t         _ss_pin;      // slave select
  uint8_t         _rst_pin;     // reset
  uint8_t         _dio0_pin;
  const uint32_t  _spi_clock;

 public:
  Rfm69();
  void begin(Hardware = RFM69HW, SPIClass* = NULL);
  void setPins(uint8_t ss_pin = NULL, uint8_t rst_pin = NULL, uint8_t d100_pin = NULL);
  void setPowerAmp(uint8_t);
  void reset();
  void writeReg(uint8_t addr, uint8_t value);
  void writeFifo(uint8_t *data, uint8_t length);
  uint8_t readReg(uint8_t addr);
  uint8_t readFifo(uint8_t *buf);
  bool isPayloadReady();
  bool waitFor(uint8_t addr, uint8_t mask, uint8_t val, uint8_t timeout=500);

  void setModulation(uint8_t data);
  void setPreamble(uint16_t data);
  void setBitrate(uint16_t bps);
  void setFrequencyDeviation(uint16_t freq);
  void setFrequency(uint32_t freq);
  void setPayloadLength(uint8_t value);
  void setFifoThreshold(uint8_t value);
  void setPacketConfig(uint8_t cfg1, uint8_t cfg2);
  void setSync(uint8_t cfg, uint32_t value);
  void setPower(uint8_t pwr);
  void setAutomode(uint8_t automode);

  void setMode(uint8_t mode);
  void setModeTx();
  void setModeRx();
  void setModeSleep();
  void setModeStandby();

  void regDump();
};

} // namespace homectrl

#endif

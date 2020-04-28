#include <Arduino.h>
#include <SPI.h>

#include "homectrl_rfm69.h"

namespace homectrl {

#define RADIO_SPI        VSPI
#define RADIO_SPI_MISO   MISO
#define RADIO_SPI_MOSI   MOSI
#define RADIO_SPI_SCLK   SCK
#define RADIO_SPI_SS     SS

#define RADIO_RESET      4
#define RADIO_D100       15

//uninitalised pointers to SPI objects
SPIClass *_spi = NULL;
SPISettings _spi_settings;


void Rfm69::initialize() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RADIO_RESET, OUTPUT);
  pinMode(RADIO_SPI_SS, OUTPUT);
  digitalWrite(RADIO_SPI_SS, HIGH);

  _spi = new SPIClass(RADIO_SPI);
  _spi->begin();
  _spi_settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);

  this->reset();
}

void Rfm69::reset() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(RADIO_RESET, HIGH);
  delay(150);
  digitalWrite(RADIO_RESET, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
}

void Rfm69::writeReg(uint8_t addr, uint8_t value) {
  _spi->beginTransaction(_spi_settings);
  digitalWrite(RADIO_SPI_SS, LOW); //pull SS slow to prep other end for transfer

  _spi->transfer(addr | 0x80);
  _spi->transfer(value);

  digitalWrite(RADIO_SPI_SS, HIGH); //pull ss high to signify end of data transfer
  _spi->endTransaction();
}

void Rfm69::writeFifo(uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    this->writeReg(REG_FIFO, data[i]);    
  }
}

uint8_t Rfm69::readReg(uint8_t addr) {
  _spi->beginTransaction(_spi_settings);
  digitalWrite(RADIO_SPI_SS, LOW); //pull SS slow to prep other end for transfer

  _spi->transfer(addr & 0x7f);
  uint8_t val = _spi->transfer(0);
    
  digitalWrite(RADIO_SPI_SS, HIGH); //pull ss high to signify end of data transfer
  _spi->endTransaction();  

  return val;
}

uint8_t Rfm69::readFifo(uint8_t *buf) {
  uint8_t count = 0;

  while (this->readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_FIFONOTEMPTY) {
    buf[count] = this->readReg(REG_FIFO);
    count++;
  }
  return count;
}

bool Rfm69::isPayloadReady() {
  if (this->readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY) {
    return true;
  }
  return false;
}

bool Rfm69::waitFor(uint8_t addr, uint8_t mask, uint8_t val, uint8_t timeout) {
  while (timeout >= 0) {
    uint8_t ret = this->readReg(addr);
    if ((ret & mask) == (val ? mask : 0)) return true;
    delay(1);
    timeout--;    
  }
  return false;
}

void Rfm69::setModulation(uint8_t data) {
  this->writeReg(REG_DATAMODUL, data);
}

void Rfm69::setPreamble(uint16_t data) {
  this->writeReg(REG_PREAMBLEMSB, data >> 8 & 0xff);
  this->writeReg(REG_PREAMBLELSB, data & 0xff);
}

void Rfm69::setBitrate(uint16_t bps) {
  this->writeReg(REG_BITRATEMSB, bps >> 8 & 0xff);
  this->writeReg(REG_BITRATELSB, bps & 0xff);
}

void Rfm69::setFrequencyDeviation(uint16_t freq) {
  this->writeReg(REG_FDEVMSB, freq >> 8 & 0xff);
  this->writeReg(REG_FDEVLSB, freq & 0xff);
}

void Rfm69::setFrequency(uint32_t freq) {
  this->writeReg(REG_FRFMSB, freq >> 16 & 0xff);
  this->writeReg(REG_FRFMID, freq >> 8 & 0xff);
  this->writeReg(REG_FRFLSB, freq & 0xff);
}

void Rfm69::setPayloadLength(uint8_t value) {
  this->writeReg(REG_PAYLOADLENGTH, value);
}

void Rfm69::setFifoThreshold(uint8_t value) {
  this->writeReg(REG_FIFOTHRESH, value);
}

void Rfm69::setPacketConfig(uint8_t cfg1, uint8_t cfg2) {
  this->writeReg(REG_PACKETCONFIG1, cfg1);
  this->writeReg(REG_PACKETCONFIG2, cfg2);
}

void Rfm69::setSync(uint8_t cfg, uint32_t value) {
  this->writeReg(REG_SYNCCONFIG, cfg);
  this->writeReg(REG_SYNCVALUE1, value >> 24 & 0xff);
  this->writeReg(REG_SYNCVALUE2, value >> 16 & 0xff);
  this->writeReg(REG_SYNCVALUE3, value >> 8 & 0xff);
  this->writeReg(REG_SYNCVALUE4, value >> 0 & 0xff);  
}

void Rfm69::setPower(uint8_t pwr) {
  this->writeReg(REG_PALEVEL, pwr);
}

void Rfm69::setAutomode(uint8_t automode) {
  this->writeReg(REG_AUTOMODES, automode);
}

void Rfm69::setMode(uint8_t mode) {
  this->writeReg(REG_OPMODE, mode);
}

void Rfm69::setModeTx() {
  this->setMode(RF_OPMODE_TRANSMITTER);
}

void Rfm69::setModeRx() {
  this->setMode(RF_OPMODE_RECEIVER);
}

void Rfm69::setModeSleep() {
  this->setMode(RF_OPMODE_SLEEP);
}

void Rfm69::setModeStandby() {
  this->setMode(RF_OPMODE_STANDBY);
}

void Rfm69::regDump() {
  char str[16];
  
  sprintf(str, "%02X", this->readReg(REG_PALEVEL));
  Serial.print("PALEVEL:       ");
  Serial.println(str);

  sprintf(str, "%02X%02X", this->readReg(REG_FDEVMSB), this->readReg(REG_FDEVLSB));
  Serial.print("FDEV:          ");
  Serial.println(str);

  Serial.print("RXBW:          ");
  Serial.println(this->readReg(REG_RXBW));

  sprintf(str, "%02X%02X%02X", this->readReg(REG_FRFMSB), this->readReg(REG_FRFMID), this->readReg(REG_FRFLSB));
  Serial.print("FRF:           ");
  Serial.println(str);

  sprintf(str, "%02X %02X", this->readReg(REG_IRQFLAGS1), this->readReg(REG_IRQFLAGS2));
  Serial.print("IRQFLAGS:      ");
  Serial.println(str);
  
  sprintf(str, "%02X", this->readReg(REG_SYNCCONFIG));
  Serial.print("SYNCCONFIG:    ");
  Serial.println(str);

  sprintf(str, "%02X %02X", this->readReg(REG_SYNCVALUE1), this->readReg(REG_SYNCVALUE1));
  Serial.print("SYNCVALUE:     ");
  Serial.println(str);

  sprintf(str, "%02X %02x", this->readReg(REG_PACKETCONFIG1), this->readReg(REG_PACKETCONFIG2));
  Serial.print("PACKETCONFIG:  ");
  Serial.println(str);
}

} // namespace homectrl

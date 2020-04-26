#include <Arduino.h>
#include <RFM69registers.h>
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


void rfm69_initialize() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RADIO_RESET, OUTPUT);
  pinMode(RADIO_SPI_SS, OUTPUT);
  digitalWrite(RADIO_SPI_SS, HIGH);

  _spi = new SPIClass(RADIO_SPI);
  _spi->begin();
  _spi_settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);

  rfm69_reset();
}

void rfm69_reset() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(RADIO_RESET, HIGH);
  delay(150);
  digitalWrite(RADIO_RESET, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
}

void rfm69_write_reg(uint8_t addr, uint8_t value) {
  _spi->beginTransaction(_spi_settings);
  digitalWrite(RADIO_SPI_SS, LOW); //pull SS slow to prep other end for transfer

  _spi->transfer(addr | 0x80);
  _spi->transfer(value);

  digitalWrite(RADIO_SPI_SS, HIGH); //pull ss high to signify end of data transfer
  _spi->endTransaction();
}

void rfm69_write_fifo(uint8_t *data, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    rfm69_write_reg(REG_FIFO, data[i]);    
  }
}

uint8_t rfm69_read_reg(uint8_t addr) {
  _spi->beginTransaction(_spi_settings);
  digitalWrite(RADIO_SPI_SS, LOW); //pull SS slow to prep other end for transfer

  _spi->transfer(addr & 0x7f);
  uint8_t val = _spi->transfer(0);
    
  digitalWrite(RADIO_SPI_SS, HIGH); //pull ss high to signify end of data transfer
  _spi->endTransaction();  

  return val;
}

uint8_t rfm69_read_fifo(uint8_t *buf) {
  uint8_t count = 0;

  while (rfm69_read_reg(REG_IRQFLAGS2) & RF_IRQFLAGS2_FIFONOTEMPTY) {
    buf[count] = rfm69_read_reg(REG_FIFO);
    count++;
  }
  return count;  
}

bool rfm69_is_payload_ready() {
  if (rfm69_read_reg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY) {
    return true;
  }
  return false;
}

bool rfm69_wait_for(uint8_t addr, uint8_t mask, uint8_t val, uint8_t timeout) {
  while (timeout >= 0) {
    uint8_t ret = rfm69_read_reg(addr);
    if ((ret & mask) == (val ? mask : 0)) return true;
    delay(1);
    timeout--;    
  }
  return false;
}

void rfm69_set_modulation(uint8_t data) {
  rfm69_write_reg(REG_DATAMODUL, data);
}

void rfm69_set_preamble(uint16_t data) {
  rfm69_write_reg(REG_PREAMBLEMSB, data >> 8 & 0xff);
  rfm69_write_reg(REG_PREAMBLELSB, data & 0xff);
}

void rfm69_set_bitrate(uint16_t bps) {
  rfm69_write_reg(REG_BITRATEMSB, bps >> 8 & 0xff);
  rfm69_write_reg(REG_BITRATELSB, bps & 0xff);
}

void rfm69_set_frequency_deviation(uint16_t freq) {
  rfm69_write_reg(REG_FDEVMSB, freq >> 8 & 0xff);
  rfm69_write_reg(REG_FDEVLSB, freq & 0xff);
}

void rfm69_set_frequency(uint32_t freq) {
  rfm69_write_reg(REG_FRFMSB, freq >> 16 & 0xff);
  rfm69_write_reg(REG_FRFMID, freq >> 8 & 0xff);
  rfm69_write_reg(REG_FRFLSB, freq & 0xff);
}

void rfm69_set_payload_length(uint8_t value) {
  rfm69_write_reg(REG_PAYLOADLENGTH, value);
}

void rfm69_set_fifo_threshold(uint8_t value) {
  rfm69_write_reg(REG_FIFOTHRESH, value);
}

void rfm69_set_packet_config(uint8_t cfg1, uint8_t cfg2) {
  rfm69_write_reg(REG_PACKETCONFIG1, cfg1);
  rfm69_write_reg(REG_PACKETCONFIG2, cfg2);
}

void rfm69_set_sync(uint8_t cfg, uint32_t value) {
  rfm69_write_reg(REG_SYNCCONFIG, cfg);
  rfm69_write_reg(REG_SYNCVALUE1, value >> 24 & 0xff);
  rfm69_write_reg(REG_SYNCVALUE2, value >> 16 & 0xff);
  rfm69_write_reg(REG_SYNCVALUE3, value >> 8 & 0xff);
  rfm69_write_reg(REG_SYNCVALUE4, value >> 0 & 0xff);  
}

void rfm69_set_power(uint8_t pwr) {
  rfm69_write_reg(REG_PALEVEL, pwr);
}

void rfm69_set_automode(uint8_t automode) {
  rfm69_write_reg(REG_AUTOMODES, automode);
}

void rfm69_set_mode(uint8_t mode) {
  rfm69_write_reg(REG_OPMODE, mode);
}

void rfm69_set_mode_tx() {
  rfm69_set_mode(RF_OPMODE_TRANSMITTER);
}

void rfm69_set_mode_rx() {
  rfm69_set_mode(RF_OPMODE_RECEIVER);
}

void rfm69_set_mode_sleep() {
  rfm69_set_mode(RF_OPMODE_SLEEP);
}

void rfm69_set_mode_standby() {
  rfm69_set_mode(RF_OPMODE_STANDBY);
}

void rfm69_reg_dump() {
  char str[16];
  
  sprintf(str, "%02X", rfm69_read_reg(REG_PALEVEL));
  Serial.print("PALEVEL:       ");
  Serial.println(str);

  sprintf(str, "%02X%02X", rfm69_read_reg(REG_FDEVMSB), rfm69_read_reg(REG_FDEVLSB));
  Serial.print("FDEV:          ");
  Serial.println(str);

  Serial.print("RXBW:          ");
  Serial.println(rfm69_read_reg(REG_RXBW));

  sprintf(str, "%02X%02X%02X", rfm69_read_reg(REG_FRFMSB), rfm69_read_reg(REG_FRFMID), rfm69_read_reg(REG_FRFLSB));
  Serial.print("FRF:           ");
  Serial.println(str);

  sprintf(str, "%02X %02X", rfm69_read_reg(REG_IRQFLAGS1), rfm69_read_reg(REG_IRQFLAGS2));
  Serial.print("IRQFLAGS:      ");
  Serial.println(str);
  
  sprintf(str, "%02X", rfm69_read_reg(REG_SYNCCONFIG));
  Serial.print("SYNCCONFIG:    ");
  Serial.println(str);

  sprintf(str, "%02X %02X", rfm69_read_reg(REG_SYNCVALUE1), rfm69_read_reg(REG_SYNCVALUE1));
  Serial.print("SYNCVALUE:     ");
  Serial.println(str);

  sprintf(str, "%02X %02x", rfm69_read_reg(REG_PACKETCONFIG1), rfm69_read_reg(REG_PACKETCONFIG2));
  Serial.print("PACKETCONFIG:  ");
  Serial.println(str);
}

} // namespace homectrl

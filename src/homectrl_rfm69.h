#ifndef _HOMECTRL_RFM69_H_
#define _HOMECTRL_RFM69_H_

namespace homectrl {

#define RF_FDEV_5000       0x0052

#define RF_BITRATE_1200    0x682B
#define RF_BITRATE_2400    0x3415
#define RF_BITRATE_4800    0x1A0B
#define RF_BITRATE_9600    0x0D05
#define RF_BITRATE_19200   0x0683
#define RF_BITRATE_38400   0x0341
#define RF_BITRATE_57600   0x022C


void rfm69_initialize();
void rfm69_reset();
void rfm69_set_mode_rx();
void rfm69_write_reg(uint8_t addr, uint8_t value);
void rfm69_write_fifo(uint8_t *data, uint8_t length);
uint8_t rfm69_read_reg(uint8_t addr);
uint8_t rfm69_read_fifo(uint8_t *buf);
bool rfm69_is_payload_ready();
bool rfm69_wait_for(uint8_t addr, uint8_t mask, uint8_t val, uint8_t timeout=500);

void rfm69_set_modulation(uint8_t data);
void rfm69_set_preamble(uint16_t data);
void rfm69_set_bitrate(uint16_t bps);
void rfm69_set_frequency_deviation(uint16_t freq);
void rfm69_set_frequency(uint32_t freq);
void rfm69_set_payload_length(uint8_t value);
void rfm69_set_fifo_threshold(uint8_t value);
void rfm69_set_packet_config(uint8_t cfg1, uint8_t cfg2);
void rfm69_set_sync(uint8_t cfg, uint32_t value);
void rfm69_set_power(uint8_t pwr);
void rfm69_set_automode(uint8_t automode);
void rfm69_set_mode(uint8_t mode);
void rfm69_set_mode_tx();
void rfm69_set_mode_rx();
void rfm69_set_mode_sleep();
void rfm69_set_mode_standby();
void rfm69_reg_dump();

} // namespace homectrl

#endif

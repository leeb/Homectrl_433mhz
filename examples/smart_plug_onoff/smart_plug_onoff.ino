/**
 * Switch a legacy Energenie MIHO002 adapter on and off
 */

#include <Arduino.h>
#include <homectrl_433mhz.h>

#define DEVICE_ID 0x123456

homectrl::Rfm69 rfm69;
homectrl::Energenie energenie;

void setup() {
  Serial.begin(115200);

  // optional function to override the default pins for SS, RST and DIO0
  // rfm69.setPins(NULL, NULL, NULL);

  // Initializes the RFM69 and SPI connection. 
  // An SPIClass can be passed to override the default when the hardware supports multiple interfaces.
  rfm69.initialize();

  // Now pass it to the energenie instance
  energenie.setRfm69(rfm69);
}

void loop() {
  Serial.println("Turn device on");
  energenie.modeTransmit();
  energenie.sendState(0x0, DEVICE_ID);
  delay(100);
  energenie.modeReceive();

  delay(3000);

  Serial.println("Turn device off");
  energenie.modeTransmit();
  energenie.sendState(0x1, DEVICE_ID);
  delay(100);
  energenie.modeReceive();

  delay(3000);
}

  

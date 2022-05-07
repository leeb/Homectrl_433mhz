/**
 * Switch a legacy Energenie MIHO002 adapter on and off
 */

#include <Arduino.h>
#include <homectrl_433mhz.h>

// replace with ID of your own device.
#define DEVICE_ID 0x123456
//#define DEVICE_ID 0x00372f  // test adapter

homectrl::Rfm69 rfm69;
homectrl::Energenie energenie;

void on_energenie_msg(homectrl::EnergenieMsg *msg) {
  Serial.println("Energenie packet decoded");
  Serial.print("Address: ");
  Serial.println(msg->address, HEX);
  Serial.print("State: ");
  Serial.println(msg->state, HEX);
}


void setup() {
  Serial.begin(115200);

  // optional function to override the default pins for SS, RST and DIO0
  // rfm69.setPins(NULL, NULL, NULL);

  // Initializes the RFM69 and SPI connection. 
  // Specify the hardware type here so that the power amplifiers are set correctly.
  // An SPIClass can be passed to override the default when the hardware supports multiple interfaces.
  rfm69.begin(homectrl::Rfm69::RFM69HW);

  // Now pass it to the energenie instance
  energenie.begin(rfm69);
  energenie.modeReceive();

  energenie.onEnergenieMsg(on_energenie_msg);

  Serial.println("Ready...");
}


void loop() {
  static int count;

  if (rfm69.isPayloadReady()) {
    Serial.println("Payload ready");
    energenie.receivePayload();
  }  

  if ((count & 0x03ff) == 0x0080) {
    Serial.println("Turn device on");
    energenie.modeTransmit();
    energenie.sendState(0x0, DEVICE_ID);
    energenie.modeReceive();    
  }

  if ((count & 0x03ff) == 0x0280) {
    Serial.println("Turn device off");
    energenie.modeTransmit();
    energenie.sendState(0x1, DEVICE_ID);
    energenie.modeReceive();
  }
  
  count++;
  delay(10);
}

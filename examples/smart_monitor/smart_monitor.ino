/**
 * Listens for messages from Energenie MiHome devices such as MIHO004 & MIHO005
 */

#include <Arduino.h>
#include <homectrl_433mhz.h>

homectrl::Rfm69 rfm69;
homectrl::MiHome mihome;

void setup() {
  Serial.begin(115200);

  // optional function to override the default pins for SS, RST and DIO0
  // rfm69.setPins(NULL, NULL, NULL);

  // Initializes the RFM69 and SPI connection. 
  // An SPIClass can be passed to override the default when the hardware supports multiple interfaces.
  rfm69.begin();

  // Now pass it to the mihome instance
  mihome.begin(rfm69);

  // Let's connect the callback events for each message
  // back to the internal function which will dump their
  // their contents out to the serial interface.
  
  mihome.onMonitorMsg([](homectrl::MiHomeMonitorMsg *msg) {
    mihome.dumpMessage(msg);
  });

  mihome.onAdapterPlusMsg([](homectrl::MiHomeAdapterPlusMsg *msg) {
    mihome.dumpMessage(msg);
  });

  mihome.onSwitchStateMsg([](homectrl::MiHomeSwitchStateMsg *msg) {
    mihome.dumpMessage(msg);    
  });

  // Put the RFM69 into receive mode for MiHome messages
  mihome.modeReceive();
}

void loop() {
  if (rfm69.isPayloadReady()) {
    Serial.println("*** Payload ready ***");
    mihome.receivePayload();
  }
  delay(10);
}

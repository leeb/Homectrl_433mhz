/**
 * Switches on and off a MIHO005 smart plug plus, while monitoring its messages.
 */

// replace with ID of your own device.
#define DEVICE_ID 0x123456

#include <Arduino.h>
#include <homectrl_433mhz.h>

homectrl::Rfm69 rfm69;
homectrl::MiHome mihome;


void setup() {
  Serial.begin(115200);

  rfm69.begin();
  mihome.begin(rfm69);

  // Instead of using the internal dump function,
  // we inspect the messages directly in this examples.

  mihome.onAdapterPlusMsg([](homectrl::MiHomeAdapterPlusMsg *msg) {
    if (msg->sensor_id == DEVICE_ID) {
      Serial.print("Power:");
      Serial.println(msg->switch_state);
    }
  });

  mihome.onSwitchStateMsg([](homectrl::MiHomeSwitchStateMsg *msg) {
    if (msg->sensor_id == DEVICE_ID) {
      Serial.print("Power:");
      Serial.println(msg->switch_state);
    }
  });  

  mihome.modeReceive();
}


/**
 * Alternating every five seconds will send three on command and three off commands.
 * Polls for message payloads on each loops.
 */
void loop() {
  static uint16_t i = 0;

  if (rfm69.isPayloadReady()) {
    mihome.receivePayload();
  }

  switch (i & 0x3ff) {
    case 0:
      Serial.println("Turn device on");
    case 50:
    case 100:
      mihome.modeTransmit();
      mihome.sendSwitchState(0x1, DEVICE_ID);
      mihome.modeReceive();
      break;

    case 512:
      Serial.println("Turn device off");
    case 552:
    case 602:
      mihome.modeTransmit();
      mihome.sendSwitchState(0x0, DEVICE_ID);
      mihome.modeReceive();
      break;
  }
  
  delay(10);  
  i++;
}

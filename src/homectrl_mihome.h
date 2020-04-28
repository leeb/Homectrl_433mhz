#ifndef _HOMECTRL_MIHOME_H_
#define _HOMECTRL_MIHOME_H_

#include <functional>

#include "homectrl_rfm69.h"
#include "homectrl_openthings.h"


namespace homectrl {

#define MANUFACTURER_MIHOME         0x04

#define MIHOME_MONITOR              0x01
#define MIHOME_ADAPTER_PLUS         0x02


typedef struct MiHomeMonitorMsg : OpenThingsHeader {
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  bool      join;  
} MiHomeMonitorMsg;

typedef struct MiHomeAdapterPlusMsg : OpenThingsHeader {
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  uint8_t   switch_state;
  bool      join;  
} MiHomeAdapterPlusMsg;

typedef struct MiHomeSwitchStateMsg : OpenThingsHeader {
  uint8_t   switch_state;
} MiHomeSwitchStateMsg;



class OpenThingsMsg {
 public:
  uint8_t   size;
  uint8_t   manufacturer_id;
  uint8_t   product_id;
  uint16_t  reserved;
  uint32_t  sensor_id;

  virtual void dump();
  virtual void serialize(char *buf);
  void decodeHeader(uint8_t *buf);
};

/*
class MihomeMonitorMsg : public OpenThingsMsg {
 public:
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  bool      join;

  void dump();
  // void serialize(char *buf);    
  void decode(uint8_t *buf, uint8_t len);
};

class MihomeAdapterPlusMsg : public OpenThingsMsg {
 public:
  int16_t   real_power;
  int16_t   reactive_power;
  uint8_t   voltage;
  uint16_t  frequency;
  uint8_t   switch_state;
  bool      join;  

  void dump();
  void serialize(char *buf);    
  void decode(uint8_t *buf, uint8_t len);
};

class MihomeSwitchStateMsg : public OpenThingsMsg {
 public:
  uint8_t   switch_state;

  void dump();
  void decode(uint8_t *buf, uint8_t len);
};
*/

class MiHome {
 private:
  Rfm69     *rfm69;

 public:
  MiHome();
  void setRfm69(Rfm69&);
  void modeTransmit();
  void modeReceive();
  uint8_t receivePayload();

  void transmitPayload(uint8_t *buf, uint8_t len);
  void sendSwitchState(uint8_t switch_state, uint32_t sensor_id, uint8_t product_id=MIHOME_ADAPTER_PLUS, uint8_t manufacturer=MANUFACTURER_MIHOME);

  void dumpHeader(OpenThingsHeader*);
  void dumpMessage(MiHomeAdapterPlusMsg*);
  void dumpMessage(MiHomeSwitchStateMsg*);
  void dumpMessage(MiHomeMonitorMsg*);
  
  void onMsg(std::function<void(OpenThingsHeader*)> cb) { cb_openthings_msg = cb; };
  void onSwitchStateMsg(std::function<void(MiHomeSwitchStateMsg*)> cb) { cb_switch_state_msg = cb; };
  void onAdapterPlusMsg(std::function<void(MiHomeAdapterPlusMsg*)> cb) { cb_adapter_plus_msg = cb; };
  void onMonitorMsg(std::function<void(MiHomeMonitorMsg*)> cb) { cb_monitor_msg = cb; };

 private:
  void receiveHeader(uint8_t*, OpenThingsHeader*);
  void receiveSwitchState(uint8_t*, uint8_t);
  void receiveAdapterPlus(uint8_t*, uint8_t);
  void receiveMonitor(uint8_t*, uint8_t);

  void unassigned() { return; };

  std::function<void(OpenThingsHeader*)> cb_openthings_msg;
  std::function<void(MiHomeSwitchStateMsg*)> cb_switch_state_msg;
  std::function<void(MiHomeAdapterPlusMsg*)> cb_adapter_plus_msg;
  std::function<void(MiHomeMonitorMsg*)> cb_monitor_msg = {};
};

} // namespace homectrl

#endif

/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */


#ifndef MyGateway_h
#define MyGateway_h

#ifndef USE_DELEGATES
  #define USE_DELEGATES
#endif

#include "Libraries/MySensors/MyConfig.h"
#include "Libraries/MySensors/MySensor.h"
#include "Libraries/MySensors/MyTransport.h"
#include "Libraries/MySensors/MyTransportNRF24.h"
#include "Libraries/MySensors/MyHwESP8266.h"

#define EEPROM_LATEST_NODE_ADDRESS ((uint8_t)EEPROM_LOCAL_CONFIG_ADDRESS)
#define GW_FIRST_SENSORID 20      // If you want manually configured nodes below
                                  // this value. 255 = Disable
#define GW_LAST_SENSORID  254     // 254 is max! 255 reserved.
#define GW_UNIT           "M"     // Select M for metric or I for imperial.
#define S_FIRSTCUSTOM     60

typedef struct sensor
{
    uint8_t node;
    uint8_t sensor;
    uint8_t type;
    String value;
} sensor_t;
#define MAX_MY_SENSORS 32

typedef Delegate<void(const MyMessage &)> msgRxDelegate;
typedef Delegate<void(int sensorId, String value)> sensorValueChangedDelegate;

class MyGateway
{
  public:
	MyGateway(MyTransport &radio =*new MyTransportNRF24(),
                  MyHw &hw=*new MyHwDriver()
#ifdef MY_SIGNING_FEATURE
		, MySigning &signer=*new MySigningNone()
#endif
#ifdef WITH_LEDS_BLINKING
		, uint8_t _rx=DEFAULT_RX_LED_PIN,
		uint8_t _tx=DEFAULT_TX_LED_PIN,
		uint8_t _er=DEFAULT_ERR_LED_PIN,
		unsigned long _blink_period=DEFAULT_LED_BLINK_PERIOD
#endif
		);

	void begin(msgRxDelegate = NULL,
                   sensorValueChangedDelegate valueChanged = NULL,
                   uint64_t base_address = RF24_BASE_RADIO_ID);
        const char * version();
        boolean sendRoute(MyMessage &msg);
        MyMessage& build (MyMessage &msg, uint8_t destination,
                          uint8_t sensor, uint8_t command,
                          uint8_t type, bool enableAck);
        void registerHttpHandlers(HttpServer &server);
        static String getSensorTypeString(int type);
        static int getSensorTypeFromString(String type);

  protected:
    void process();
    void incomingMessage(const MyMessage &message);
    void onGetSensors(HttpRequest &request,
                      HttpResponse &response);
    void onRemoveSensor(HttpRequest &request,
                        HttpResponse &response);

  private:
    char convBuf[MAX_PAYLOAD*2+1];
    msgRxDelegate msgRx;
    sensorValueChangedDelegate sensorValueChanged;
    MySensor gw;
    Timer processTimer;
    bool nodeIds[256];
    sensor_t mySensors[MAX_MY_SENSORS];
    MyMessage msg;
};

#endif
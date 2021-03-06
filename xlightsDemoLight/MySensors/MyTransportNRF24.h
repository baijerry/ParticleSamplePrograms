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

#ifndef MyTransportNRF24_h
#define MyTransportNRF24_h

#include "application.h"

#include "MyConfig.h"
#include "MyMessage.h"
#include "MyTransport.h"
#include "particle-rf24.h"

// Address based on customized netwoork id, SBS updated 2016-06-28
//#define TO_ADDR(x) (RF24_BASE_RADIO_ID + x)
#define TO_ADDR(cust, x) ((cust) + x)

#define WRITE_PIPE ((uint8_t)0)
#define CURRENT_NODE_PIPE ((uint8_t)0)
#define BROADCAST_PIPE ((uint8_t)1)
#define PRIVATE_NET_PIPE ((uint8_t)2)

class MyTransportNRF24 : public MyTransport
{
public:
	MyTransportNRF24(uint8_t ce=RF24_CE_PIN, uint8_t cs=RF24_CS_PIN, uint8_t paLevel=RF24_PA_LEVEL);
	bool init();
	void setAddress(uint8_t address, uint64_t network);
	uint8_t getAddress();
	bool send(uint8_t to, const void* data, uint8_t len, uint8_t pipe = 255);
	bool send(uint8_t to, MyMessage &message, uint8_t pipe = 255);
	bool available(uint8_t *to, uint8_t *pipe = NULL);
	uint8_t receive(void* data);
	void powerDown();

	// SBS added 2016-06-28
	uint64_t getCurrentNetworkID() const;
	uint64_t getMyNetworkID() const;
	bool switch2BaseNetwork();
	bool switch2MyNetwork();
	bool isValid();
	void PrintRFDetails();

	// SBS added 2016-07-22
	void enableBaseNetwork(bool sw = true);
	bool isBaseNetworkEnabled() { return _bBaseNetworkEnabled; };

private:
	RF24 rf24;
	uint8_t _address;
	uint8_t _paLevel;

	// SBS added 2016-06-28
	uint64_t _currentNetworkID;
	uint64_t _myNetworkID;
	bool _bValid;

	// SBS added 2016-07-22
	bool _bBaseNetworkEnabled;
};

#endif

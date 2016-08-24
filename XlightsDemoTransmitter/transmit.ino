/*
<<<<<< MySensor Interface Documentation >>>>>>
  Sensor_ID: S_DIMMER
  Subtypes: V_STATUS, V_DIMMER
      S_DIMMER represents the light as a whole. It has 2 variables, V_STATUS
      and V_DIMMER. V_STATUS=0 or =1 turns all 3 rings on/off. V_DIMMER turns
      all 3 rings to the specified brightness.

  Sensor: S_CUSTOM
  Var: V_VAR1
      S_CUSTOM represents the individual rings. All hue information is contained
      in V_VAR1, which is type uint64_t (8 bytes)

      V_VAR1 Structure (little endian):
      --------------------------
      Byte #     data
      --------------------------
      7       ring # (0 = all rings, 1 = ring1, 2 = ring2... etc)
      6       Status (1/on or 0/off)
      5       Cold white value
      4       Warm white value
      3       Red value
      2       Green value
      1       Blue value
<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>
*/

//mySensor Defines
#define MY_RADIO_NRF24
#define MY_DEBUG


#include "application.h"
#include "MyParserSerial.h"
#include "MyTransportNRF24.h"

// Maximun JSON data length
#define COMMAND_JSON_SIZE         64
#define SENSORDATA_JSON_SIZE      196

//pins

//instance of class
MyTransportNRF24 radio(A0, A2);
MyParserSerial msgParser;
MyMessage msg;

//global var
UC *msgData = (UC *)&(msg.msg);
uint64_t networkID;
UC nodeID = 0;
int brightness = 0;

//forward declares
bool cldProcessSend(String &strMsg);
int easytest_ring1_color (String color);
void send_message (MyMessage msg);

void setup()
{
  Serial.begin(115200);

  //cld declares
  Particle.function("SerialMesg", cldSerialMyMessageSend);
  Particle.function("easytest_r1", easytest_ring1_color);

  networkID = RF24_BASE_RADIO_ID;

  //mysensor inits
  radio.init();


  if( !radio.init() ) {
    Serial.println("Radio is not valid!");
    return;
  };

  //networkID = GetNetworkID();
  radio.setAddress(GATEWAY_ADDRESS, networkID);
  //radio.enableBaseNetwork(true);
  //radio.switch2BaseNetwork();
}

void loop()
{
}

int easytest_ring1_color (String color)
{
  color.toLowerCase();
  uint8_t to_nodeID = 1;
  if (color.equals("red"))
  {
    //282574505050112
    msg.build(nodeID, to_nodeID, S_CUSTOM, C_SET, V_VAR1, true);
    uint64_t payload = 282574505050112;
    msg.set(payload);
  }
  else if (color.equals("green"))
  {
    //282574488403712
    msg.build(nodeID, to_nodeID, S_CUSTOM, C_SET, V_VAR1, true);
    uint64_t payload = 282574488403712;
    msg.set(payload);
  }
  else if (color.equals("blue"))
  {
    //282574488338687
    msg.build(nodeID, to_nodeID, S_CUSTOM, C_SET, V_VAR1, true);
    uint64_t payload = 282574488338687;
    msg.set(payload);
  }

  send_message(msg);
}

uint64_t GetNetworkID()
{
  uint64_t netID = 0;

  byte mac[6];
  WiFi.macAddress(mac);
  for (int i=2; i<6; i++) {
    netID += mac[i];
    netID <<= 8;
  }

  return netID;
}

bool cldSerialMyMessageSend(String strMsg)
{
  // test cases

  // Set S_CUSTOM:
    // all rings red: 1;23;1;1;24;1099528339456
    // all rings CW/WW: 1;23;1;1;24;2199006478336
    // all rings off: 1;23;1;1;24;16777215
    // ring 1 red: 1;23;1;1;24;282574505050112
  // Set S_DIMMER, V_STATUS on/off
    // 	1;4;1;1;2;1
  // Set S_DIMMER, V_DIMMER value 0-100
    //  1;4;1;1;3;50

  int iValue;
  char strBuffer[64];
  iValue = min(strMsg.length(), 63);
  strncpy(strBuffer, strMsg.c_str(), iValue);
  strBuffer[iValue] = 0;

  if( msgParser.parse(msg, strBuffer) )
  {
    Serial.println("Input parsed correctly");
  }
  else
  {
    Serial.println("Bad input");
    return false;
  }

  send_message(msg);
}

void send_message (MyMessage msg)
{
  uint8_t to = 1;
  bool successflag = false;
  uint8_t destination_nodeid = msg.getDestination();

  successflag = radio.send(destination_nodeid, msg);



  if (successflag)
  {
    Serial.printlnf("Sending message: from:%d to:%d dest:%d cmd:%d type:%d sensor:%d payl-len:%d",
        msg.getSender(), to, msg.getDestination(), msg.getCommand(),
        msg.getType(), msg.getSensor(), msg.getLength());
  }
  else
  {
    Serial.println("Failed to send RF message.");
  }
}

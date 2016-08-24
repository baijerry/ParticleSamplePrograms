/*
This is a demo of the smart lighting fixture. It will take
RF input from the SmartController, and change the color of
its tri-colour LEDS. There will be 3 rings inside, which will
be cycled through.

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

      V_VAR1 Structure:
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
#include "MyTransportNRF24.h"
#include "MyMessage.h"
#include "MyParserSerial.h"

// Maximun JSON data length
#define COMMAND_JSON_SIZE         64
#define SENSORDATA_JSON_SIZE      196

//pins
int pin_red = A1; //rgb led red pin
int pin_green = A6; //rgb led green pin
int pin_blue = A7; //rgb led blue pin
int pin_r1 = D0; //led lights up when ring1 color is shown
int pin_r2 = D1; //led lights up when ring2 color is shown
int pin_r3 = D2; //led lights up when ring3 color is shown

//instance of class
MyTransportNRF24 radio(A0, A2);
MyMessage msg;

//global var
typedef enum {BLACK,
              RED,
              GREEN,
              BLUE,
              YELLOW,
              AQUA } COLOR;

COLOR ring1_old;
COLOR ring2_old;
COLOR ring3_old;

COLOR ring1;
COLOR ring2;
COLOR ring3;

int cld_ring1;
int cld_ring2;
int cld_ring3;

UC *msgData = (UC *)&(msg.msg);
String mLastMessage;

uint64_t networkID;
UC nodeID = 1;
int brightness = 0;

//forward declares
bool ProcessReceive(); //recieve and print radio payload
void ProcessCommand(MyMessage msg); //act on radio payload (toggle lights, change brightness etc)
uint64_t GetNetworkID();
void parse_payload(uint64_t payload, uint8_t &ring, bool &state, uint8_t &CW, uint8_t &WW, uint8_t &R, uint8_t &G, uint8_t &B);

//demo specific forward declares
void demotoggleLights(uint8_t ring, bool state, uint8_t R, uint8_t G, uint8_t B);
COLOR maptocolor(uint8_t R, uint8_t G, uint8_t B);
void changeColor(COLOR color);
void setColorPins(uint8_t r, uint8_t g, uint b);
void resetRGB();

void setup()
{
  Serial.begin(115200);
  setDefaults();

  //pins
  pinMode(pin_red, OUTPUT);
  pinMode(pin_green, OUTPUT);
  pinMode(pin_blue, OUTPUT);
  pinMode(pin_r1, OUTPUT);
  pinMode(pin_r2, OUTPUT);
  pinMode(pin_r3, OUTPUT);

  networkID = RF24_BASE_RADIO_ID;

  //mysensor inits
  radio.init();

  //Particle.function("CloudCmd", CloudCommand);
  Particle.variable("LastMessage", &mLastMessage, STRING);
  Particle.variable("Brightness", &brightness, INT);
  Particle.variable("cld_ring1", cld_ring1);
  Particle.variable("cld_ring2", cld_ring2);
  Particle.variable("cld_ring3", cld_ring3);

  Particle.function("V_VAR1-test", testpayload_V_VAR1);
  Particle.function("PrintRFInfo", print_rf_info);

  if( !radio.init() ) {
    mLastMessage = "Radio is not valid!";
    Serial.println("Radio is not valid!");
    SERIAL_LN(mLastMessage);
    return;
  };

  //networkID = GetNetworkID();
  radio.setAddress(nodeID, networkID);
}

void setDefaults()
{
  ring1 = RED;
  ring2 = BLUE;
  ring3 = YELLOW;

  ring1_old = RED;
  ring2_old = BLUE;
  ring3_old = YELLOW;
}

void loop()
{
  if( radio.isValid() )
  {
    ProcessReceive();
  }
  else
  {
    mLastMessage = "radio not available";
    Serial.println("radio not available");
  }

  int mDelay = 1000;
  //ring1
  digitalWrite(pin_r1, HIGH);
  changeColor(ring1);
  delay(mDelay);
  digitalWrite(pin_r1, LOW);
  resetRGB();
  //ring2
  digitalWrite(pin_r2, HIGH);
  changeColor(ring2);
  delay(mDelay);
  digitalWrite(pin_r2, LOW);
  resetRGB();
  //ring1
  digitalWrite(pin_r3, HIGH);
  changeColor(ring3);
  delay(mDelay);
  digitalWrite(pin_r3, LOW);
  resetRGB();

  cld_ring1 = (int)ring1;
  cld_ring2 = (int)ring2;
  cld_ring3 = (int)ring3;
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

bool ProcessReceive()
{
  bool sentOK = false;
  uint8_t to = 0;
  uint8_t pipe;

  if (!radio.available(&to, &pipe))
    return false;

  uint8_t len = radio.receive(msgData);
  if (len < HEADER_SIZE) {
    Serial.println("corrupt data, bad header length");
    return false;
  }

  Serial.printlnf("Received from pipe %d mes-leg=%d, from: %d, to:%d, dest:%d, cmd:%d, type:%d, sensor:%d, payl-len: %d",
    pipe, len, msg.getSender(), to, msg.getDestination(), msg.getCommand(),
    msg.getType(), msg.getSensor(), msg.getLength());

  ProcessCommand(msg);

  return true;
}

void ProcessCommand(MyMessage msg)
{
  switch (msg.getCommand())
  {
    case C_PRESENTATION:
      Serial.println("Error command msgType");
      break;

    case C_SET:
      if (msg.getSensor() == S_CUSTOM)
      {
        if (msg.getType() == V_VAR1)
        {
          uint64_t payload = msg.getUInt64();

          bool state;
          UC ring, CW, WW, R, G, B;
          parse_payload(payload, ring, state, CW, WW, R, G, B);
          char buf[12];
          PrintUint64(buf, payload, false);
          Serial.printlnf("Color payload recieved: %s", buf);
          Serial.printlnf("ring: %u", ring);
          Serial.printlnf("state: %u", state);
          Serial.printlnf("Cold White: %u", CW);
          Serial.printlnf("Warm White: %u", WW);
          Serial.printlnf("Red: %u", R);
          Serial.printlnf("Green: %u", G);
          Serial.printlnf("Blue: %u", B);

          //switch light stuff
          demotoggleLights(ring, state, R, G, B);
        }
        else
        {
          Serial.println("Error, wrong subtype.");
        }
      }
      else if (msg.getSensor() == S_DIMMER)
      {
        if (msg.getType() == V_STATUS)
        {
          bool light_status = msg.getInt();
          if (light_status == false)
          {
            Serial.printlnf("Light Status: off");
            ring1_old = ring1;
            ring2_old = ring2;
            ring3_old = ring3;
            ring1 = BLACK;
            ring2 = BLACK;
            ring3 = BLACK;
          } else {
            //get last known values of all rings, set rings to on
            ring1 = ring1_old;
            ring2 = ring2_old;
            ring3 = ring3_old;
            Serial.printlnf("Light Status: on");
          }
        }
        else if (msg.getType() == V_DIMMER)
        {
          int light_brightness = msg.getInt();
          Serial.printlnf("Light Brightness: %d", light_brightness);
        }
        else
        {
          Serial.println("Error, wrong subtype.");
        }
      }
      else
      {
        Serial.println("Error, wrong sensor id.");
      }
      break;

    case C_REQ:
      break;

    case C_INTERNAL:
      break;

    case C_STREAM:
      break;
  }
}

void parse_payload(uint64_t payload, uint8_t &ring, bool &state, uint8_t &CW, uint8_t &WW, uint8_t &R, uint8_t &G, uint8_t &B)
{
  ring = (payload >> (8*6)) & 0xff;
  state = (payload >> (8*5)) & 0xff;
  CW = (payload >> (8*4)) & 0xff;
  WW = (payload >> (8*3)) & 0xff;
  R = (payload >> (8*2)) & 0xff;
  G = (payload >> (8*1)) & 0xff;
  B = (payload >> (8*0)) & 0xff;
}

int print_rf_info (String in)
{
  radio.PrintRFDetails();
}

//============================================================
// Demo Specific Hardware Related Functions
//============================================================
int testpayload_V_VAR1(String in)
{
  //testing values
  //282574488403967    <- 0x 01 01 00 00 00 FF FF
  //
    /*
    char in[17];
    int i = 0;
    while (Serial.available() > 0 && i < 17)
    {
      in[i] = Serial.read();
      Serial.print(in[i]);
      i++;
    }
    Serial.println("");
    */
    uint64_t input;
    input = strtoull(in.c_str(), NULL, 0);

    MyMessage outMsg;
    outMsg.build(AUTO, BASESERVICE_ADDRESS, S_CUSTOM, C_SET, V_VAR1, false);
    outMsg.set(input);

    ProcessCommand(outMsg);
}

void demotoggleLights(uint8_t ring, bool state, uint8_t R, uint8_t G, uint8_t B)
{
  if (ring == 0) //allrings
  {
    if (state) { //on
      //change all ring colors
      ring1 = maptocolor(R, G, B);
      ring2 = maptocolor(R, G, B);
      ring3 = maptocolor(R, G, B);
    } else {
      //set all ring to black
      ring1_old = ring1;
      ring2_old = ring2;
      ring3_old = ring3;
      ring1 = BLACK;
      ring2 = BLACK;
      ring3 = BLACK;
    }
  }
  else if (ring == 1){
    if (state) { //on
      //change ring colors
      ring1 = maptocolor(R, G, B);
    } else {
      //set ring to black
      ring1_old = ring1;
      ring1 = BLACK;
    }
  }
  else if (ring == 2){
    if (state) { //on
      //change ring colors
      ring2 = maptocolor(R, G, B);
    } else {
      //set ring to black
      ring2_old = ring2;
      ring2 = BLACK;
    }
  }
  else if (ring == 3){
    if (state) { //on
      //change ring colors
      ring3 = maptocolor(R, G, B);
    } else {
      //set ring to black
      ring3_old = ring3;
      ring3 = BLACK;
    }
  }
}

COLOR maptocolor(uint8_t R, uint8_t G, uint8_t B)
{
  //find largest 2 non-zero numbers
  bool flg_red = false;
  bool flg_green = false;
  bool flg_blue = false;

  if ((R >= G) && (R >= B))
  {
    flg_red = true;
    if (G > B) {
      flg_green = true;
    } else {
      flg_blue = true;
    }
  }
  else if ((G >= R) && (G >= B))
  {
    flg_green = true;
    if (R > B) {
      flg_red = true;
    } else {
      flg_blue = true;
    }
  }
  else if ((B >= G) && (B >= R))
  {
    flg_blue = true;
    if (G > R) {
      flg_green =true;
    } else {
      flg_red = true;
    }
  }

  if (R == 0) {flg_red = false;}
  if (G == 0) {flg_green = false;}
  if (B == 0) {flg_blue = false;}

  Serial.printlnf("Red flag: %d", flg_red);
  Serial.printlnf("Green flag: %d", flg_green);
  Serial.printlnf("Blue flag: %d", flg_blue);

  if (flg_red && flg_green){
    return YELLOW;
  }
  if (flg_red && flg_blue){
    return BLACK; //no purple
  }
  if (flg_green && flg_blue){
    return AQUA;
  }
  if (flg_red){
    return RED;
  }
  if (flg_green){
    return GREEN;
  }
  if (flg_blue){
    return BLUE;
  }

  return BLACK;
}

void changeColor(COLOR color)
{
  switch (color)
  {
    case BLACK:
      setColorPins(0, 0 , 0);
      break;
    case RED:
      setColorPins(255, 0 , 0);
      break;
    case GREEN:
      setColorPins(0, 255, 0);
      break;
    case BLUE:
      setColorPins(0, 0, 255);
      break;
    case YELLOW:
      setColorPins(255, 255, 0);
      break;
    case AQUA:
      setColorPins(0, 255, 255);
      break;
  }
}

void setColorPins(uint8_t r, uint8_t g, uint b)
{
  if (g >= 255) {
    digitalWrite(pin_green, HIGH);
  } else {
    analogWrite(pin_green, g);
  }

  if (r >= 255) {
    digitalWrite(pin_red, HIGH);
  } else {
    analogWrite(pin_red, r);
  }

  if (b >= 255) {
    digitalWrite(pin_blue, HIGH);
  } else {
    analogWrite(pin_blue, b);
  }
}

void resetRGB()
{
  digitalWrite(pin_red, LOW);
  digitalWrite(pin_green, LOW);
  digitalWrite(pin_blue, LOW);
  delay(100);
}

#include "application.h"
#include <math.h>

bool flag = false;

const uint8_t ring = 2;
const uint8_t state = 0;
const uint8_t CW = 200;
const uint8_t WW = 123;
const uint8_t R = 0;
const uint8_t G = 255;
const uint8_t B = 34;

#define IF_MAINLOOP_TIMER(x, name) ({unsigned long ulStart = millis(); x; Serial.printf("%s spent %lu ms", (name), millis() - ulStart);})

void setup()
{
  Serial.begin(9600);
  Particle.function("Begin", begin);
}

void loop()
{
  if (flag)
  {
    String pkg = packagepayload();
    Serial.println("Post conversion string");
    Serial.println(pkg);


    flag = false;
  }
}

int begin (String in)
{
  flag = true;
}

String packagepayload()
{
  uint64_t payload = (B / 16)* pow(16, 1)  +  (B % 16)* pow(16, 0) +
                     (G / 16)* pow(16, 3)  +  (G % 16)* pow(16, 2) +
                     (R / 16)* pow(16, 5)  +  (R % 16)* pow(16, 4) +
                     (WW / 16)* pow(16, 7)  +  (WW % 16)* pow(16, 6) +
                     (CW / 16)* pow(16, 9)  +  (CW % 16)* pow(16, 8) +
                     (state / 16)* pow(16, 11)  +  (state % 16)* pow(16, 10) +
                     (ring / 16)* pow(16, 13)  +  (ring % 16)* pow(16, 12);

  Serial.println("pre-conversion to string");
  parsepayload(payload);

  char buf[21];
  PrintUint64(buf, payload, false);
	return String(buf);
}

char* PrintUint64(char *buf, uint64_t value, bool bHex) {
  if (buf != NULL) {
    if( value > 0xFFFFFFFFLL ) {
      if( bHex ) {
        uint32_t part1 = value >> 32;
  			uint32_t part2 = value & 0xFFFFFFFFLL;
        sprintf(buf, "0x%X%04X", part1, part2);
      } else {
        uint8_t digits[20];
        uint64_t tmpV = value;
        int len = 0;
        while( tmpV > 0x00LL ) {
          digits[len++] = tmpV % 10;
          tmpV /= 10;
        }
        for( int i = len - 1; i >= 0; i-- ) {
          buf[len-1-i] = digits[i] + '0';
        }
        buf[len] = NULL;
      }
    } else {
      if( bHex ) {
        sprintf(buf, "0x%04X", value);
      } else {
        sprintf(buf, "%d", value);
      }
    }
  }
  return buf;
}

void parsepayload(uint64_t payload)
{
  uint8_t ring2 = (payload >> (8*6)) & 0xff;
  uint8_t state2 = (payload >> (8*5)) & 0xff;
  uint8_t CW2 = (payload >> (8*4)) & 0xff;
  uint8_t WW2 = (payload >> (8*3)) & 0xff;
  uint8_t R2 = (payload >> (8*2)) & 0xff;
  uint8_t G2 = (payload >> (8*1)) & 0xff;
  uint8_t B2 = (payload >> (8*0)) & 0xff;

  Serial.printlnf("ring: %u", ring2);
  Serial.printlnf("state: %u", state2);
  Serial.printlnf("Cold White: %u", CW2);
  Serial.printlnf("Warm White: %u", WW2);
  Serial.printlnf("Red: %u", R2);
  Serial.printlnf("Green: %u", G2);
  Serial.printlnf("Blue: %u", B2);

}

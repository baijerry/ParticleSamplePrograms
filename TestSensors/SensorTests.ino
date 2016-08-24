#include "application.h"
#include "Adafruit_DHT.h"
#include "LightSensor.h"


/* --------- VARS --------- */
enum Sensor {_MIC, _DHT, _ALS, _PIR, _NONE};
Sensor sensor = _NONE;
int const MIC_PIN = A6;
int const DHT_PIN = A1;
int const ALS_PIN = A7;
int const PIR_PIN = D7;
//MIC VARS
const int sampleWindow = 500; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
//DHT VARS
DHT dhtsensor(DHT_PIN, DHT11);
//ALS VARS
LightSensor alssensor(ALS_PIN);
//PIR VARS
int pirState = LOW;
int ledPin = A5;
int val = 0;
long unsigned int currentlow=0;
long unsigned int lastlow=0;


/* --------- PROTOTYPES --------- */
void setupMIC();
void setupDHT();
void setupALS();
void setupPIR();
void readMIC();
void readDHT();
void readALS();
void readPIR();
int pickSensor(String input);
void readSensor(Sensor sensor);


/* --------- SETUP --------- */
void setup(){
  Particle.function("pickSensor", pickSensor);
  Serial.begin(9600);

  //init
  setupMIC();
  setupDHT();
  setupALS();
  setupPIR(); //wait 10sec for PIR
}


/* --------- MAIN LOOP --------- */
void loop(){
  readSensor(sensor);
  delay(2000);
}


/* --------- FUNCTIONS --------- */
void setupMIC(){

}

void setupDHT(){

}

void setupALS(){
  alssensor.begin(0, 3720);
  //this contains a pinMode init for ALS_PIN. If you call pinMode() more than once
  //for 1 pin, a bug in the particle firmware will result in incorrect values
}

void setupPIR(){
  pinMode(ledPin, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  Serial.println("Init PIR, wait 10 sec");
  delay(10000);
}

void readMIC() {
  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
     sample = analogRead(MIC_PIN);
     if (sample < 1024)  // toss out spurious readings
     {
        if (sample > signalMax)
        {
           signalMax = sample;  // save just the max levels
        }
        else if (sample < signalMin)
        {
           signalMin = sample;  // save just the min levels
        }
     }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

  Serial.println("MIC: ");
  Serial.println(volts);

  char MICString[8];
  sprintf(MICString, "MIC: %f", volts);
  Particle.publish("sensorTests/MIC", MICString);
}

void readDHT() {
  Serial.println("DHT: ");
  Serial.println(dhtsensor.getTempCelcius());
}

void readALS(){
  Serial.println("ALS: ");
  //Serial.println(analogRead(ALS_PIN));
  Serial.println(alssensor.getLevel());
}

void readPIR(){
  val = digitalRead(PIR_PIN);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    digitalWrite(ledPin, HIGH);  // turn LED ON
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("PIR: Motion detected");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    digitalWrite(ledPin, LOW); // turn LED OFF
    if (pirState == HIGH){
      Serial.println("PIR: Motion terminated");
      pirState = LOW;
    }
  }
}

int pickSensor(String input) {
  if (strcmp(input, "MIC") == 0){
        sensor = _MIC;
    } else if (strcmp(input, "DHT") == 0){
        sensor = _DHT;
    } else if (strcmp(input, "ALS") == 0){
        sensor = _ALS;
    } else if (strcmp(input, "PIR") == 0){
        sensor = _PIR;
    }
    return 1;
  }

void readSensor(Sensor sensor) {
    switch(sensor){
      case _MIC:
        readMIC();
        break;
      case _DHT:
        readDHT();
        break;
      case _ALS:
        readALS();
        break;
      case _PIR:
        readPIR();
        break;
      case _NONE:
        break;
    }
  }

//1. Actuators (Line 6)
//2. Sensors (Line 43)
//3. Controller
//4. Setup & Loop

#include <Arduino.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define MAXSMOKE 20
#define MAXCARBON 20
#define SNOOZETIME 250 //time in seconds
#define MAXREADINGS 32768

#define SMOKEPIN A0
#define COPIN A1
#define SIRENPIN A2
#define TEMPPIN 12
#define LEDPIN 13

///////////////////////Actuators////////////////////////////////////

class LEDController {
    const int sPin = LEDPIN;

  public:
    LEDController() {
    }

    bool activate() {
      digitalWrite(sPin, HIGH);
      return true;
    }

    bool deactivate() {
      digitalWrite(sPin, LOW);
      return true;
    }

};

class SirenController {
    const int sPin = SIRENPIN;
  public:
    SirenController() {
      pinMode(sPin, OUTPUT);
    }

    void activate() {
      tone(sPin, 1000, 200);
      delay(100);
    }

    void deactivate() {
      noTone(sPin);
    }

};

///////////////////////Sensors////////////////////////////////////

class SmokeDetector {
    float num;
    const int sPin = SMOKEPIN;
    int val = 0;
    float voltage = 0.0;
    int sensorThres = 400;
  public:
    SmokeDetector() {
      Serial.begin(9600);
      pinMode(sPin, INPUT);
    }

    int getReading() {
      return analogRead(sPin);
    }

};

class TempSensor {
    const int A = 6 * 4;
    const int B = 64 * 4;
    const int C = 60 * 4;
    const int D = 10 * 4;
    const int E = 9 * 4;
    const int F = 55 * 4;
    const int G = 0;
    const int H = 480 * 4;
    const int I = 70 * 4;
    const int J = 410 * 4;
    //insert pin for the sensor:
    const int sPin = TEMPPIN;
  public:
    TempSensor() {
    }

    void tickDelay(int tick) {
      delayMicroseconds(tick / 4); //tick is supposed to be 0.25us
    }

    int oneWireReset() { //return value is 1 if the temp sensor indicated it is present
      int result;
      tickDelay(G);
      digitalWrite(sPin, 0);
      tickDelay(H);
      digitalWrite(sPin, 1);
      result = digitalRead(sPin) ^ 0x01; //^ = XOR
      tickDelay(J);
      return result;
    }

    void oneWireWriteBit(int bit) {
      if (bit) {
        digitalWrite(sPin, 0);
        tickDelay(A);
        digitalWrite(sPin, 1);
        tickDelay(B);
      } else {
        digitalWrite(sPin, 0);
        tickDelay(C);
        digitalWrite(sPin, 1);
        tickDelay(D);
      }
    }

    int oneWireReadBit() {
      int result;
      digitalWrite(sPin, 0);
      tickDelay(A);
      digitalWrite(sPin, 1);
      tickDelay(E);
      digitalRead(sPin) & 0x01;
      tickDelay(F);
    }

    void oneWireWriteByte(int data) {
      for (int i = 0; i < 8; i++) {
        oneWireWriteBit(data & 0x01);
        data >>= 1;
      }
    }

    double oneWireReadByte() {
      int result = 0;
      for (int i = 0; i < 8; i++) {
        result >>= 1;
        if (oneWireReadBit()) {
          result |= 0x80; //| is OR
        }
      }
      return result;
    }

    double getReading() {
      return oneWireReadByte();
    }

};

class CarbonDetector {

    int reading = 0; // value read from the sensor
    const int sPin = COPIN;
    bool warmedOne = false;
    bool warmedTwo = false;
  public:
    CarbonDetector() {
    }

    // Preparation

    void warmUpOne() {
      // turn the heater fully on
      analogWrite(sPin, HIGH); // HIGH = 255
      warmedOne = true;
    }

    void warmUpTwo() {
      analogWrite(sPin, 71.4); // 255x1400/5000
      warmedTwo = true;
    }

    //Reading

    double getReading() {
      if (warmedOne && warmedTwo) {
        analogWrite(sPin, HIGH);
        delay(100); // Getting an analog read apparently takes 100uSec
        reading = analogRead(sPin);
        analogWrite(sPin, LOW);
        warmedOne = false;
        warmedTwo = false;
        return reading;
      }
    }
};

///////////////////////////////////////////////////Controller//////////////////////////////////////////

class Controller {
    SmokeDetector sd;
    CarbonDetector cd;
    TempSensor ts;
    float currentReadings[3];
    float averageReadings[3];
    int readingsSaved;
    LEDController lc;
    SirenController sc;
    time_t hushEnd = time(0);

    bool lightsActivated;
    bool online;

    void getSensorData() { //Collect readings from the sensors.
      currentReadings[0] = sd.getReading();

      //Code for the CO2 sensor reading
      if (readingsSaved % 300 == 0) {
        currentReadings[1] = cd.getReading();
        if (Serial.available()) {
          Serial.print("Smoke Sensor: ");
          Serial.print(currentReadings[0]);
          Serial.print("\tCarbon Monoxide: ");
          Serial.print(currentReadings[1]);
          Serial.print("\t Temperature: ");
          Serial.println(currentReadings[2]);
        }
      } else if (readingsSaved % 200 == 0) {
        cd.warmUpTwo();
      } else if (readingsSaved % 100 == 0) {
        cd.warmUpOne();
      }

      currentReadings[2] = ts.getReading();
    }

    void logData() { //Reset the average reading values if the average includes more than MAXREADINGS readings, incorporates the new currentReadings into the average
      readingsSaved++;

      if (readingsSaved > MAXREADINGS) {
        for (int i = 0; i != 3; i++) {
          averageReadings[i] = 0;
        }
        readingsSaved = 0;
      }
      for (int i = 0; i != 3; i++) {

        //Special case for the CO sensor
        if (i == 1) {
          averageReadings[i] = (averageReadings[i] * (readingsSaved / 1500) + currentReadings[i]) / (readingsSaved / 1500);
        }

        //Regular case
        averageReadings[i] = (averageReadings[i] * readingsSaved + currentReadings[i]) / readingsSaved;
      }
    }

    void activateLights() { //Activate the lights
      lightsActivated = lc.activate();
    }

    void deactivateLights() { //Deactivate the lights
      lightsActivated = !lc.deactivate();
    }

    void activateAlarm() { //Activate the alarm
      sc.activate();
    }

    void deactivateAlarm() { //Deactivate the alarm
      sc.deactivate();
    }

    boolean isSnoozed() { //Checks to see if the the current time in milliseconds is larger than the end of the snooze period (HushEnd)
      time_t currentTime = time(0);
      int difference = hushEnd - currentTime;
      if (difference < 0) {
        return false;
      } else {
        return true;
      }
    }

    boolean interpretData() { //Check to see if the smoke and CO readings are below the set maximums. If they exceed the set maximums, the alarm and lights will be triggered.
      if ((currentReadings[0] > MAXSMOKE) && (currentReadings[1] > MAXCARBON) && !isSnoozed()) {
        activateAlarm();
        activateLights();
      }
    }

    void hushAlarm() { //Disables the alarm and lights, sets HushEnd to SNOOZETIME milliseconds in the future
      hushEnd = time(0);
      hushEnd = hushEnd + SNOOZETIME;
      deactivateAlarm();
      deactivateLights();
    }
  public:
    void loop() {
      while (true) {
        getSensorData();
        logData();
        interpretData();
        delay(10);
      }
    }
    Controller() {
      hushEnd = time(0);
    }
};
void setup() {
  //digitalWrite(13, HIGH);
  Controller c;
  c.loop();
}

void loop() {
  digitalWrite(13, LOW);
  //digitalWrite(1, LOW);
}

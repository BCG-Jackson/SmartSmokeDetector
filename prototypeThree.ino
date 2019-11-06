#include <Time.h>
#include <TimeLib.h>

//1. Actuators (Line 6)
//2. Sensors (Line 43)
//3. Controller
//4. Setup & Loop

#include <Arduino.h>
#include <stdio.h>
#include <unistd.h>
//#include <Time.h>
//#include <ctime>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MAXSMOKE 150
#define MAXCARBON 0//800
#define SNOOZETIME 300 //time in seconds, 5 minutes
#define MAXREADINGS 32768

#define SMOKEPIN A0
#define COPIN A1
#define SIRENPIN A2
#define TEMPPIN 12
#define LEDPIN 8

//Global variables for the temperature sensor since it wouldn't work when placed in the TempSensor class
OneWire oneWire(TEMPPIN);
DallasTemperature sensors(&oneWire);

///////////////////////Actuators////////////////////////////////////

class LEDController {
    const int sPin = LEDPIN;

  public:
    LEDController() {
    }

    bool activate() {
      analogWrite(sPin, HIGH);
      return true;
    }

    bool deactivate() {
      analogWrite(sPin, LOW);
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
      delay(20);
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
  public:
    TempSensor() {
      sensors.begin();  // Start up the library
    }

  public:
    double getReading() {
      sensors.requestTemperatures();
      return ((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);      
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
      pinMode(sPin, OUTPUT);
      analogWrite(sPin, HIGH); // HIGH = 255
      warmedOne = true;
    }

    void warmUpTwo() {
      pinMode(sPin, OUTPUT);
      analogWrite(sPin, 71.4); // 255x1400/5000
      warmedTwo = true;
    }

    //Reading

    double getReading() {
      if (warmedOne && warmedTwo) {
        pinMode(sPin, INPUT);
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
    time_t hushEnd = now();

    bool lightsActivated = false;
    bool sirenActivated = false;
    bool online;

    void getSensorData() { //Collect readings from the sensors.
      currentReadings[0] = sd.getReading();

      //Code for the CO2 sensor reading
      if (readingsSaved % (MAXREADINGS - 20) == 0) {
        currentReadings[1] = cd.getReading();
      } else if (readingsSaved % (2* MAXREADINGS / 3) == 0) {
        cd.warmUpTwo();
      } else if (readingsSaved % (MAXREADINGS / 3) == 0) {
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
      Serial.print("Activating!");
    }

    void deactivateLights() { //Deactivate the lights
      lightsActivated = !lc.deactivate();
    }

    void activateAlarm() { //Activate the alarm
      sirenActivated = true;
    }

    void deactivateAlarm() { //Deactivate the alarm
      sirenActivated = false;
    }

    boolean isSnoozed() { //Checks to see if the the current time in milliseconds is larger than the end of the snooze period (HushEnd)
      time_t currentTime = now();
      int difference = hushEnd - currentTime;
      Serial.println(difference);
      if (difference <= 0) {
        return false;
      } else {
        Serial.println("I'm Snoozed!");
        return true;
      }
    }

    boolean interpretData() { //Check to see if the smoke and CO readings are below the set maximums. If they exceed the set maximums, the alarm and lights will be triggered.
//      if (((currentReadings[0] > MAXSMOKE) || (currentReadings[1] > MAXCARBON)) && !isSnoozed()) {
//        if(readingsSaved >= 100 && (currentReadings[2] - averageReadings[2])
//        activateAlarm();
//        activateLights();
//      }else {
//        
//      }

      //If the current smoke reading is over the threshold AND the temperature has increased more than 3 degrees in the past minute
      if((currentReadings[0] > MAXSMOKE) && (currentReadings[2] - averageReadings[2] > 3)){
        activateAlarm();
        activateLights(); 
      }else if(currentReadings[1] >= MAXCARBON){
          activateAlarm();
          activateLights();
        }else{
          deactivateAlarm();
          deactivateLights();
        }
      }
      //If the CO reading is over the threshold
    

    void hushAlarm() { //Disables the alarm and lights, sets HushEnd to SNOOZETIME milliseconds in the future
      hushEnd = now();
      hushEnd = hushEnd + SNOOZETIME;
      deactivateAlarm();
      deactivateLights();
    }
  public:
    void loop() {
      while (true) {
        getSensorData();
        logData();
        if((readingsSaved % 60) == 0){
        interpretData();
        }
        if (Serial.available()) {
          Serial.print("Smoke Sensor: ");
          Serial.print(currentReadings[0]);
          Serial.print("\tCarbon Monoxide: ");
          Serial.print(currentReadings[1]);
          Serial.print("\tTemperature: ");
          Serial.println(currentReadings[2]);
        }
        if(sirenActivated){
          sc.activate();
        }
        delay(500);//This makes it read about once per second
      }
    }
    Controller() {
      hushEnd = now();
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

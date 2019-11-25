#include <Time.h>
#include <TimeLib.h>
#include <Thread.h>

#include <SPI.h>
#include <WiFiNINA.h>

#define ppp client.println

char ssid[] = "WSU_EZ_Connect";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

//1. Actuators (Line 6)
//2. Sensors (Line 43)
//3. Controller
//4. Setup & Loop

#include <Arduino.h>
#include <stdio.h>
#include <unistd.h>
//#include <ctime>
#include <OneWire.h>
#include <DallasTemperature.h>

// constants

#define MAXSMOKE 150
#define MAXCARBON 200//800
#define HUSHTIME 300 //time in seconds, 5 minutes
#define MAXREADINGS 61

// pins

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
      pinMode(sPin,OUTPUT);
    }

    bool activate() {
      digitalWrite(sPin, LOW);
      Serial.println("Turning LED on");
      return true;
    }

    bool deactivate() {
      digitalWrite(sPin, HIGH);
      Serial.println("Turning LED off");
      return false;
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
    public:
    float currentReadings[3];
    public:
    float averageReadings[3];
    int readingsSaved;
    LEDController lc;
    SirenController sc;

    time_t hushEnd = now();
    bool hushing = false;

    bool lightsActivated = false;
    bool sirenActivated = false;

    int setupCount = 0;

  public:
    Controller() {
      hushEnd = now();
      currentReadings[1] = 0;
    }
    
    void getSensorData() { //Collect readings from the sensors.
      currentReadings[0] = sd.getReading();

      //Code for the CO2 sensor reading
      if (readingsSaved % (MAXREADINGS - 20) == 0) {
        currentReadings[1] = cd.getReading();
      } else if (readingsSaved % (2 * MAXREADINGS / 3) == 0) {
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

    /// lights / LED ///

    void toggleLights() {
      
      //hushing=false; // override hush
      Serial.print("LightsActivated = ");
      Serial.println(lightsActivated);
      if(lightsActivated){
        deactivateLights();
      }else{
        activateLights();
      }
    }

    void activateLights() { //Activate the lights
      lightsActivated = lc.activate();
    }

    void deactivateLights() { //Deactivate the lights
     lightsActivated = lc.deactivate();
    }

    String getOppositeLEDState() {
      if (lightsActivated == true) {
        return "off";
      } else
      {
        return "on";
      }
    }

    /// siren / alarm ///

    void toggleAlarm() {
      hushing=false; // override hush
      if (sirenActivated == true) {
        deactivateAlarm();
        Serial.println("Turning alarm off");
      } else
      {
        activateAlarm();
        Serial.println("Turning alarm on");
      }
    }

    void activateAlarm() { //Activate the alarm
      sirenActivated = true;
    }

    void deactivateAlarm() { //Deactivate the alarm
      sirenActivated = false;
    }

    String getOppositeSirenState() {
      if (sirenActivated == true) {
        return "off";
      } else
      {
        return "on";
      }
    }

    // this is the function that triggers the alarms
    boolean interpretData() { 
      if(hushing){
        return;
      }

      //If the current smoke reading is over the threshold AND the temperature has increased more than 3 degrees in the past minute
      if ((currentReadings[0] > MAXSMOKE)){ //&& (currentReadings[2] - averageReadings[2] > 3)) {
        Serial.println("Smoke detected!");
        activateAlarm();
        activateLights();
        // if current carbon monoxide is greater than max allowed
      } else if (currentReadings[1] >= MAXCARBON) {
        Serial.println("Carbon monoxide detected!");
        activateAlarm();
        activateLights();
      } else {
        deactivateAlarm();
        deactivateLights();
      }
    }
    //If the CO reading is over the threshold


    void hushAlarm() { //Disables the alarm and lights, sets HushEnd to HUSHTIME milliseconds in the future
      hushing = true;
      hushEnd = now() + HUSHTIME;
      deactivateAlarm();
      deactivateLights();
    }
    
    void loop() {
        getSensorData();
        logData();
        if((readingsSaved % 60) == 0){
          interpretData();
        }
        // put something in the com input to get these to print
        if (Serial.available() > 0) {
          Serial.print("Smoke Sensor: ");
          Serial.print(currentReadings[0]);
          Serial.print("\tCarbon Monoxide: ");
          Serial.print(currentReadings[1]);
          Serial.print("\tTemperature: ");
          Serial.println(currentReadings[2]);
        }
        if(sirenActivated){
          sc.activate();
          Serial.println("would have activated siren");
        }
        // update hush. if the end of the hushing period is in the past, hushing is done
        if(now() > hushEnd) {
          hushing = false;
        } else {
          Serial.println("Hushing for "+String(hushEnd - now(), DEC)+" more seconds");
        }
     }
};

//////////////////////////////////////////////////////Server/////////////////////////////////////
class HostServer {
    Controller *controller;
    int attempts = 0;
    public:
    void SetupServer(Controller *cont) {
      Serial.println("");
      Serial.println("Attempting to setup server");
      Serial.begin(9600);      // initialize serial communication
      controller = cont;
      controller->setupCount = controller->setupCount +1;
      //Serial.println("what the fuck setups: " + String(controller->setupCount, DEC));
      // check for the WiFi module:
      if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        while (true);
      }

      String fv = WiFi.firmwareVersion();
      if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
      }

      // attempt to connect to Wifi network:
      while (status != WL_CONNECTED) {
        attempts++;
        Serial.print("Attempting to connect to Network named: ");
        Serial.println(ssid);                   // print the network name (SSID);
        Serial.println("Attempt # " + String(attempts, DEC));
        
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid);
        // wait 2 seconds for connection:
        delay(2000);
      }
      server.begin();                           // start the web server on port 80
      printWifiStatus();                        // you're connected now, so print out the status
    }



    void loop() {
      WiFiClient client = server.available();   // listen for incoming clients

      if (client) {                             // if you get a client,
        Serial.println("new client");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
          if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            if (c == '\n') {                    // if the byte is a newline character

              // if the current line is blank, you got two newline characters in a row.
              // that's the end of the client HTTP request, so send a response:
              if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println(); // this is needed

                ppp("<!DOCTYPE html>");
                ppp("<head>");
                ppp("<style>body {background-color:#00e7ff52;} a {color:orange;}</style>");
                ppp("<style>div {border-style: groove; padding: 10px; } </style>");
                ppp("<style>button {padding: 20px; width: 100%; } </style>");
                ppp("<style>p {padding-top: 10px; display: block; white-space: pre; font-family: Consolas;} </style>");
                ppp("</head>");
                ppp("<body>");
                ppp("<div style=\"float: left; width: 30%; overflow: hidden;\">");
                ppp("<button type=\"button\" onclick=\"window.location.href = \'/toggleSpotlight\'\"> Turn spotlight " + controller->getOppositeLEDState() + "</button><br>");
                //ppp("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
                ppp("<button type=\"button\" onclick=\"window.location.href = \'/toggleAlarm\'\"> Turn alarm " + controller->getOppositeSirenState() + "</button><br>");
                ppp("<button type=\"button\" onclick=\"window.location.href = \'/hush\'\"> Hush alarm for "+String(HUSHTIME, DEC)+" seconds "+"</button><br>");
                ppp("<button type=\"button\" onclick=\"window.location.href = \'/refresh\'\"> Refresh </button><br>");
                ppp("</div>");
                ppp("<div style=\"overflow: hidden;\">");
                ppp("<p>Temperature:         " + String(controller->currentReadings[2], DEC) + " &#176;C</p>");
                ppp("<p>Carbon Monoxide:    " + String(controller->currentReadings[1], DEC) + " parts per million</p>");
                ppp("<p>Smoke:              " + String(controller->currentReadings[0], DEC) + " parts per million</p>");
                ppp("</div>");
                if(controller->hushing){
                  ppp("<p>Hushing alarms for "+String(controller->hushEnd - now(), DEC)+" seconds</p>");
                }

                ppp("</body>");
                ppp("</html>");
                //

                client.println(); // The HTTP response ends with another blank line:
                break;
              } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
              }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
              currentLine += c;      // add it to the end of the currentLine
            }
            if (currentLine.endsWith("GET /toggleSpotlight")) {
              controller->toggleLights();
              Serial.println("Toggling Spotlight");
            }
            else if (currentLine.endsWith("GET /toggleAlarm")) {
              controller->toggleAlarm();
            }
            else if (currentLine.endsWith("GET /hush")) {
              controller->hushAlarm();
            }else if (currentLine.endsWith("GET /refresh")) {
              currentLine = "";
              //client.stop();
              loop();
            }

          }
        }
        // close the connection:
        client.stop();
        Serial.println("client disonnected");
      }
    }

  void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(ip);
  }


};
////////////////////////////Setup & Loop////////////////////////

//void setup() {
//  Controller c;
//  HostServer hs;
//  hs.SetupServer(&c);
//  while (true) {
//    c.loop();
//    hs.UpdateHTML();
//    delay(500);//This makes it read about once per second
//  }
//}

Controller c;
Thread offlineThread = Thread();
HostServer hs;
Thread serverThread = Thread();

void offlineCallback(){
  c.loop();
}

void serverCallback(){
  hs.loop();
}

void setup() {
  Serial.begin(9600);

  //Configure the Controller
  offlineThread.onRun(offlineCallback);
  offlineThread.setInterval(100);//1500
  
  //Configure the Server
  hs.SetupServer(&c);
  serverThread.onRun(serverCallback);
  serverThread.setInterval(500);

}

void loop() {
  if(offlineThread.shouldRun()){
    offlineThread.run();
  }else if(serverThread.shouldRun()){
    serverThread.run();
  }else{
    delay(50);
  }
}

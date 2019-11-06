
#include <SPI.h>
#include <WiFiNINA.h>



#define ppp client.println

char ssid[] = "WSU_EZ_Connect";

int status = WL_IDLE_STATUS;
WiFiServer server(80);
class Controller{};

void ffff(Controller a){}

void SetupServer() {
  Serial.begin(9600);      // initialize serial communication

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
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(2000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}



void UpdateHTML(Controller cont) {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
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
    ppp("<style>button {padding: 20px; width: 200px; } </style>");
    ppp("<style>p {padding: 10px; display: block; white-space: pre; font-family: Consolas;} </style>");
  ppp("</head>");
  ppp("<body>");
    ppp("<div>");
      ppp("<button type=\"button\" onclick=\"window.location.href = \'/toggleSpotlight\'\"> Turn spotlight " + cont.getOppositeLEDState() + "</button><br>");
      //ppp("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
      ppp("<button type=\"button\" onclick=\"window.location.href = \'/toggleAlarm\'\"> Turn alarm " + cont.getOppositeSirenState() + "</button><br>");
      
    ppp("</div>");
    ppp("<div>");
      ppp("<p>Temerature:         "+ String(cont.currentReading[2], DEC) +" &#176;C</p>");
      ppp("<p>Carbon Monoxide:    "+ String(cont.currentReading[1], DEC) +" parts per million</p>");
      ppp("<p>Smoke:              "+ String(cont.currentReading[0], DEC) +" parts per million</p>");
    ppp("</div>");

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
          cont.toggleSpotlight();
        }
        else if (currentLine.endsWith("GET /toggleAlarm")) {
          cont.toggleAlarm();
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
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

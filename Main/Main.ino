

//globals - pins
const int LED1_WIFI_CONNECTED_out  = 13;
const int SPOTLIGHT_out = 8;

const int CO_in     = A1;
const int TEMP_in   = 12;
const int SMOKE_in  = A2;

// state variables hold the last record of the sensor's data
bool spotlightIsOn = false;
int CO_state;
int TEMP_state;
int SMOKE_state;

void setup() {
  // initialize pins.
  pinMode(LED1_WIFI_CONNECTED_out, OUTPUT);
  pinMode(SPOTLIGHT_out, OUTPUT);
  pinMode(CO_in, INPUT);
  pinMode(TEMP_in, INPUT);
  pinMode(SMOKE_in, INPUT);

  digitalWrite(LED1_WIFI_CONNECTED_out, LOW); //set default off

  SetupServer();
}

void loop() {
  CO_state = digitalRead(CO_in);
  TEMP_state = digitalRead(TEMP_in);
  SMOKE_state = digitalRead(SMOKE_in);

  //Serial.println("CO read: " + CO_state);
  //Serial.println("Temp read: " + TEMP_state);
  //Serial.println("SMOKE read: " + SMOKE_state);

  // convert raw input into useful numbers

  // check if sensors indicate danger

  // push to mobile

  // wait a second now
  delay(1000);
  LoopServer();
}

void blinkLED(int LED,int delayMicroSeconds){
  digitalWrite(LED, HIGH);
  delay(delayMicroSeconds / 2);
  digitalWrite(LED, LOW);
  delay(delayMicroSeconds / 2);
}


String GetOppositeSpotlightState(){
  if(spotlightIsOn == true){
    return "off";
  } else
  {
    return "on";
  }
}

// TODO change to spotlight not led1
void ToggleSpotlight() {
  if(spotlightIsOn == true){
    digitalWrite(LED1_WIFI_CONNECTED_out, LOW);
    spotlightIsOn = false;
    Serial.println("Turning spotlight off");
  } else
  {
    digitalWrite(LED1_WIFI_CONNECTED_out, HIGH);
    spotlightIsOn = true;
    Serial.println("Turning spotlight on");
  }
}




  // https://drive.google.com/drive/u/0/folders/1SzFcsPdfYpvVmtk6itBgLK-uMxUNhMyh
  //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                       // wait for a second

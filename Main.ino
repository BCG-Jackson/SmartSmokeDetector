


const int LED1_out  = 1;
const int LED2_out  = 2;
const int CO_in     = 3;
const int TEMP_in   = 4;
const int SMOKE_in  = 5;

// state variables hold the last record of the sensor's data
int CO_state;
int TEMP_state;
int SMOKE_state;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize pins.
  pinMode(LED1_out, OUTPUT);
  pinMode(LED2_out, OUTPUT);
  pinMode(CO_in, INPUT);
  pinMode(TEMP_in, INPUT);
  pinMode(SMOKE_in, INPUT);

  // connect to wifi
  
}

void loop() {


  // verify wifi connection

  CO_state = digitalRead(CO_in);
  TEMP_state = digitalRead(TEMP_in);
  SMOKE_state = digitalRead(SMOKE_in);

  // convert raw input into useful numbers

  // check if sensors indicate danger

  delay(1000);

  // push to mobile
}



  // https://drive.google.com/drive/u/0/folders/1SzFcsPdfYpvVmtk6itBgLK-uMxUNhMyh
  //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  //delay(1000);                       // wait for a second

//Joseph Walters & Marcus Wills
//Prelab 3 for Lab 2
//Oct 8, 2018
float num;
float actualDuty;
float timeHigh;
float timeLow;
float dutyCalc;
float freqCalc;
int inputPin = 8;
int ledPin = 10;
int forceSensor = A2;
int val = 0;
float voltage = 0.0;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(inputPin, INPUT);
  pinMode(forceSensor, INPUT);


}

void loop() {
  //Measure Freq
  timeHigh = pulseIn(inputPin, HIGH);
  timeLow = pulseIn(inputPin, LOW);
  
  //Calculate Freq and use as delay
  freqCalc = 1 / ((timeHigh + timeLow) / 1000000.0);
  delay(freqCalc/2);
  val = analogRead(forceSensor);
  analogWrite(ledPin, (val/4)); //Duty cycle proportional to input voltage
  delay(freqCalc/2);
  analogWrite(ledPin, 0);



}

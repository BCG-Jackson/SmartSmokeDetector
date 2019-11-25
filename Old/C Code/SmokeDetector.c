public class SmokeDetector{
float num;
int sPin = 0;
int val = 0;
float voltage = 0.0;
int sensorThres = 400;

public SmokeDetector(int pin){
sPin = pin;
Serial.begin(9600);
pinMode(sPin, INPUT);
}

public int getReading() {
  int reading = analogRead(sPin);
}

}
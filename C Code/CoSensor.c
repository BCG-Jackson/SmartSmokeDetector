public class CarbonDetector{
//#define sPin A1      // Signal 
int reading = 0;   // value read from the sensor
boolean warmedOne = false;
boolean warmedTwo = false;

public CarbonDetector(int pin){
sPin = pin;
}

// Preparation
public void warmUpOne(){
    // turn the heater fully on
    analogWrite(sPin, HIGH); // HIGH = 255
	warmedOne = true;
	}
	
public void warmUpTwo(){
analogWrite(sPin, 71.4); // 255x1400/5000
warmedTwo = true;
}

//Reading
public double getReading(){
    analogWrite(sPin, HIGH); 
    delay(50); // Getting an analog read apparently takes 100uSec
    reading = analogRead(sPin); 
	return reading;
}
public class LEDController{
	const int LED_PIN;
	
	LEDController(int pin){
		LED_PIN = pin;
	}
	
	void turnOn(){
		digitalWrite(LED_PIN, HIGH);
	}
	
	void turnOff(){
		digitalWrite(LED_PIN, LOW);
	}

}
public class TempSensor{
const B = 64 * 4;
const C = 60 * 4;
const D = 10 * 4;
const E = 9 * 4;
const F = 55 * 4;
const G = 0;
const H = 480 * 4;
const I = 70 * 4;
const J = 410 * 4;
//insert pin for the sensor:
sPin = 0;

public TempSensor(int pin){
sPin = pin;
}

void tickDelay(int tick){
	delayMicroseconds(tick / 4); //tick is supposed to be 0.25us
}

int oneWireReset(){ //return value is 1 if the temp sensor indicated it is present
	int result;
	tickDelay(G);
	digitalWrite(sPin, 0);
	tickDelay(H);
	digitalWrite(sPin, 1);
	result = digitalRead(sPin) ^ 0x01; //^ = XOR
	tickDelay(J);
	return result;
}

void oneWireWriteBit(int bit){
	if(bit){
		digitalWrite(sPin, 0);
		tickDelay(A);
		digitalWrite(sPin, 1);
		tickDelay(B);
	}else{
		digitalWrite(sPin, 0);
		tickDelay(C);
		digitalWrite(sPin, 1);
		tickDelay(D);
	}
}
int oneWireReadBit(){
	int result;
	digitalWrite(sPin, 0);
	tickDelay(A);
	digitalWrite(sPin, 1);
	tickDelay(E);
	digitalRead(sPin) & 0x01;
	tickDelay(F);
}

void oneWireWriteByte(int data){
	for(int i = 0; i < 8; i++){
		oneWireWriteBit(data & 0x01);
		data >>= 1;
	}
}

double oneWireReadByte(){
	int result = 0;
	for(int i = 0; i < 8; i++){
		result >>= 1;
		if(oneWireReadBit()){
			result |= 0x80; //| is OR
		}
	}
	return result;
}

public double getReading(){
return oneWireReadByte();
}

}
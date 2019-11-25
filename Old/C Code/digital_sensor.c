//This code based on template at www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
//Modified for use on the Arduino platform
//These are numbers of *ticks*
const A = 6 * 4;
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
//TEMP_PIN = ;

void tickDelay(int tick){
	delayMicroseconds(tick / 4); //tick is supposed to be 0.25us
}

int oneWireReset(){ //return value is 1 if the temp sensor indicated it is present
	int result;
	tickDelay(G);
	digitalWrite(TEMP_PIN, LOW);
	tickDelay(H);
	digitalWrite(TEMP_PIN, HIGH);
	result = digitalRead(TEMP_PIN) ^ 0x01; //^ = XOR
	tickDelay(J);
	return result;
}

void oneWireWriteBit(int bit){
	if(bit){
		digitalWrite(TEMP_PIN, LOW);
		tickDelay(A);
		digitalWrite(TEMP_PIN, HIGH);
		tickDelay(B);
	}else{
		digitalWrite(TEMP_PIN, LOW);
		tickDelay(C);
		digitalWrite(TEMP_PIN, HIGH);
		tickDelay(D);
	}
}
int oneWireReadBit(){
	int result;
	digitalWrite(TEMP_PIN, LOW);
	tickDelay(A);
	digitalWrite(TEMP_PIN, HIGH);
	tickDelay(E);
	digitalRead(TEMP_PIN) & 0x01;
	tickDelay(F);
}

void oneWireWriteByte(int data){
	for(int i = 0; i < 8; i++){
		oneWireWriteBit(data & 0x01);
		data >>= 1;
	}
}

void oneWireWrite12Bit(long data){
	for(int i = 0; i < 12; i++){
		oneWireWriteBit(data & 0x01);
		data >>= 1;
	}
}

int oneWireReadByte(){
	int result = 0;
	for(int i = 0; i < 8; i++){
		result >>= 1;
		if(oneWireReadBit()){
			result |= 0x80; //| is OR
		}
	}
	return result;
}

long oneWireRead12Bit(){
	int result = 0;
	for(int i = 0; i < 12; i++){
		result >>= 1;
		if(oneWireReadBit()){
			result |= 0x800L; //| is OR
		}
	}
	return result; //12-bit value is stored in the last 12 bits of the result
}



























public class SirenController(int sPin){
	int Siren = sPin
	pinMode(Siren,output);
	
	
}

public activateSiren(){
	tone(Siren, 1000, 200);
	delay(100);
}

public deactivateSiren(){
	noTone(Siren);
}
#define MAXSMOKE 20
#define MAXCARBON 20
#define SNOOZETIME 100000 //time in millis
#define MAXREADINGS 32768

private SmokeDetector sd;
private CarbonDetector cd;
private TempSensor ts;
private float[] currentReadings;
prviate float[] averageReadings;
private int readingsSaved;
private LEDController lc;
private SirenController sc;
struct timespec hushEnd;

boolean lightsActivated;
boolean online;

public float[] getSensorData(){ //Collect readings from the sensors. 
currentReadings[0] = sd.getReading();

//Code for the CO2 sensor reading
if(readingsSaved%1500 == 0){
currentReadings[1] = cd.getReading();
}else if(readingsSaved%1000 == 0){
cd.warmUpTwo();
} else if(readingsSaved % 500 == 0){
cd.warmUpOne();
}

currentReadings[2] = ts.getReading();
}

public void logData(){//Reset the average reading values if the average includes more than MAXREADINGS readings, incorporates the new currentReadings into the average
averageReadings++;

if(readingsSaved > MAXREADINGS){
for(int i = 0; i != 3; i++){
averageReadings[i] = 0;
}
readingsSaved = 0;
}
for(int i = 0; i != 3; i++){

//Special case for the CO sensor
if(i == 1){
averageReadings[i] = (averageReadings[i] * (readingsSaved/1500) + currentReadings[i])/(readingsSaved/1500);
}

//Regular case
averageReadings[i] = (averageReadings[i] * readingsSaved + currentReadings[i])/readingsSaved;
}

public void activateLights(){//Activate the lights
lightsActivated = lc.activate();
}

public void deactivateLights(){ //Deactivate the lights
lightsActivated = !lc.deactivate();
}

public void activateAlarm(){ //Activate the alarm
sc.activate();
}

public void deactivateAlarm(){ //Deactivate the alarm
sc.deactivate();
}

private boolean isSnoozed(){ //Checks to see if the the current time in milliseconds is larger than the end of the snooze period (HushEnd)
struct timespec temp;
clock_gettime(CLOCK_MONOTOMIC_RAW, temp);
if(hushEnd < temp){
hushEnd = 100000000000000000000000;
return false;
}else{
return true;
}

public boolean interpretData(){ //Check to see if the smoke and CO readings are below the set maximums. If they exceed the set maximums, the alarm and lights will be triggered.
if((currentReadings[0] > MAXSMOKE) && (currentReadings[1] > MAXCARBON) && !isSnoozed()){
activateAlarm();
activateLights();
}
}

public void hushAlarm(){ //Disables the alarm and lights, sets HushEnd to SNOOZETIME milliseconds in the future
clock_gettime(CLOCK_MONOTOMIC_RAW, &hushEnd);
hushEnd = hushEnd + SNOOZETIME;
deactivateAlarm();
deactivateLights();
}

public void loop(){
getSensorData();
logData();
interpretData();
}

public void initialize(){
sd = new SmokeDetector(A0);
cd = new CarbonDetector(A1);
ts = new TempSensor(12);
lc = new LEDController(13); //This will be used for testing purposes
sc = new SirenController(A2);
}
//When the sensor and actuating classes are established, we will need to make an initialization method

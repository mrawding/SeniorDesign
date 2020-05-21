#include <EEPROM.h>

// You can change modify these
int delayTime = 5000; //time between readings in ms
int analogPin = A1; //analog pin being used
int samplesToAverage = 40; //number of samples to average over


/*
int ph4Address = 0;
int ph7Address = 5;
int ph10Address = 10;
*/

float voltageReading; //voltage from pin
float pH; //calculated pH

int slopeAddress = 0; //address to store slope
int offsetAddress = 10; //address to store offset

float slope = 0.00f; //slope of mV vs pH
float offset = 0.00f; //offset of mV vs pH

char inputString[32]; //used to read user input

enum state {
  CALIBRATE,
  RUN
};

state currentState; //state of the program

unsigned long startMillis = 0; 
unsigned long currentMillis = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Reading from memory...");
  EEPROM.get(slopeAddress, slope);
  Serial.print("Slope read as ");
  Serial.println(slope);
  EEPROM.get(offsetAddress, offset);
  Serial.print("Offset read as ");
  Serial.println(offset);

  Serial.println("Starting Sentron ISFET reading...");

  voltageReading = 0;
  currentState = RUN;

  startMillis = millis();
}

void loop() {
  //check if entering calibration
  if (readInput() > 0) {
    if (strcmp(inputString, "calibrate\n") == 0) {
      Serial.println("Entering calibration mode.");
      currentState = CALIBRATE;
    }
  }

  currentMillis = millis();
  
  if (currentState == RUN) {
    if(currentMillis - startMillis >= delayTime){
      voltageReading = readISFET();
      pH = (voltageReading - offset) / slope;
      Serial.print(voltageReading);
      Serial.print(" mV calculated as ");
      Serial.println(pH);

      startMillis = millis();
    }
  }
  else if (currentState == CALIBRATE) {
    Serial.println("Enter the slope in mV/pH.");

    while (!Serial.available());
    int tmp = readInput();
    //inputString[tmp - 1] = '\0';
    slope = atof(inputString);
    EEPROM.put(slopeAddress, slope);
    
    /*
    Serial.println("Enter stop when the pH value has stabilized");
    while (strcmp(inputString, "stop\n") != 0) {
      voltageReading = readISFET();
      Serial.println(voltageReading);
      
      readInput();
      delay(5000);
    } 

    if (strcmp(inputString, "4\n") == 0) {
      EEPROM.put(ph4Address, voltageReading);
    } else if (strcmp(inputString, "7\n") == 0) {
      EEPROM.put(ph7Address, voltageReading);
    } else if (strcmp(inputString, "10\n") == 0) {
      EEPROM.put(ph10Address, voltageReading);
    } */

    Serial.println("Enter the offset in mV.");
    while (!Serial.available());
    tmp = readInput();
    offset = atof(inputString);
    EEPROM.put(offsetAddress, offset);

    Serial.println("Exiting calibration mode.");
    currentState = RUN;
  }
}

//read input at the serial monitor
int readInput() {
  int availableBytes = 0;
  delay(50);
  if ((availableBytes = Serial.available()) > 0) {
    for (int i = 0; i < availableBytes; i++) {
      inputString[i] = Serial.read();
      delay(5);
    }
    inputString[availableBytes] = '\0';
  }
  return availableBytes;
}

//read voltage value from sensor
float readISFET() {
  float value = 0;
  for (int i = 0; i < samplesToAverage; i++) {
    value += ((float) analogRead(analogPin) * (3.3 / 1023.0)) * 1000.0;
  }
  value = value / samplesToAverage;
  return value;
}

#include <Adafruit_SleepyDog.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <MS5837.h>
#include <Adafruit_FONA.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_FONA.h>

/****************************************************************************
 CONNECTION DETAILS */

//PINS
//Fona
#define FONA_RX  1
#define FONA_TX  0
#define FONA_RST 2
#define FONA_KEY 4
//DS18B20 temp sensor
#define ONE_WIRE_BUS 14

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

//GPRS APN, username, password (blank for us)
#define FONA_APN       ""
#define FONA_USERNAME  ""
#define FONA_PASSWORD  ""

//MQTT Broker Setup
/*
#define AIO_SERVER      "broker.hivemq.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""
*/

#define AIO_SERVER      "tailor.cloudmqtt.com"
#define AIO_SERVERPORT  16049
#define AIO_USERNAME    "xgjaaqqe"
#define AIO_KEY         "BYkV4bRunWYt"


//Setup FONA MQTT class with FONA object and MQTT details
Adafruit_MQTT_FONA mqtt(&fona, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//Helper function that sets up FONA and connect to GPRS network
boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password);

//Set up feeds
Adafruit_MQTT_Publish temperature_feed = Adafruit_MQTT_Publish(&mqtt, "sensors/temperature");
Adafruit_MQTT_Publish ph_feed= Adafruit_MQTT_Publish(&mqtt, "sensors/ph");
Adafruit_MQTT_Publish pressure_feed = Adafruit_MQTT_Publish(&mqtt, "sensors/pressure");
Adafruit_MQTT_Publish conductivity_feed = Adafruit_MQTT_Publish(&mqtt, "sensors/conductivity");
Adafruit_MQTT_Publish tds_feed = Adafruit_MQTT_Publish(&mqtt, "sensors/tds");
Adafruit_MQTT_Publish dO_feed = Adafruit_MQTT_Publish(&mqtt, "sensors/dO");

// How many transmission failures in a row we're willing to be ok with before reset
uint8_t txfailures = 0;
#define MAXTXFAILURES 3

bool isFonaOn = false;

/****************************************************************************
 DS18B20 Temperature Sensor Details */
 /*

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature temp_probe(&oneWire);
*/
/****************************************************************************
 MS5837 Depth Sensor Details */
MS5837 pressure_sensor;

/****************************************************************************/

void setup() {
  //while(!Serial);
  Serial.begin(115200);

  Serial.println(F("Begin FONA MQTT Program"));

  Watchdog.reset();
  delay(5000);
  Watchdog.reset();

  //Set key pin
  pinMode(FONA_KEY, OUTPUT);
  
  // Initialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD))) {
    Serial.println("Retrying FONA");
  }

  Serial.println(F("Connected to Cellular"));

  //fona.enableGPS(true);

  Watchdog.reset();
  delay(5000);
  Watchdog.reset();

  // Start temperature probe
  //temp_probe.begin();

  // Start depth sensor
  //Wire.begin();

  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  /*
  while (!pressure_sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }
  
  pressure_sensor.setModel(MS5837::MS5837_30BA);
  pressure_sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
  */
}

//float latitude, longitude, speed_kph, heading, altitude;

void loop() {
  float temp = 21.23; /*read_temp();*/ //C
  float pH = 7.00; //pH
  float pressure = 1062.00; //mbar
  float conductivity = 1.41; //ms/cm
  uint32_t tds = 17; //ppm
  float dO = 7.21; // mg/L

  // read depth
  //pressure_sensor.read();
  //float pressure = pressure_sensor.pressure();
/*
  bool gps_success = false;
  //add timeout
  while(gps_success == false) {
    if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)){
      gps_success = true;
    }
    
  }
  Serial.print("GPS: "); Serial.print(latitude); Serial.print(" "); Serial.println(longitude);
  */
  Watchdog.reset();

  //Ensure connected to MQTT server. Will reconnect if not connected
  MQTT_connect();

  Watchdog.reset();
  
  //Send data
  Serial.print(F("\nSending Temperature: "));
  Serial.print(temp);
  Serial.print("...");
  if(!temperature_feed.publish((double) temp)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Serial.print(F("\nSending pH: "));
  Serial.print(pH);
  Serial.print("...");
  if(!ph_feed.publish(pH)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Serial.print(F("\nSending pressure: "));
  Serial.print(pressure);
  Serial.print("...");
  if(!pressure_feed.publish(pressure)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Serial.print(F("\nSending conductivity: "));
  Serial.print(conductivity);
  Serial.print("...");
  if(!conductivity_feed.publish(conductivity)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Serial.print(F("\nSending TDS: "));
  Serial.print(tds);
  Serial.print("...");
  if(!tds_feed.publish(tds)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Serial.print(F("\nSending dissolved oxygen: "));
  Serial.print(dO);
  Serial.print("...");
  if(!dO_feed.publish(dO)) {
    Serial.println(F("Failed"));
    txfailures++;
  }
  else {
    Serial.println(F("OK"));
    txfailures = 0;
  }

  Watchdog.reset();

  // turn off Fona
  //fona.enableGPS(false);
  Serial.println("Turning Fona Off...");
  digitalWrite(FONA_KEY, 0);
  delay(2000);
  digitalWrite(FONA_KEY, 1);
  isFonaOn = false;
  
  delay(30000);

  // Reinitialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD))) {
    Serial.println("Retrying FONA");
  }
  //fona.enableGPS(true);
}
/*
float read_temp() {
  temp_probe.requestTemperatures(); // Send the command to get temperatures

  float tempC = temp_probe.getTempCByIndex(0);
  
  // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature read: ");
    Serial.println(tempC);
    return tempC;
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
    return 0;
  }

}
*/
  
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
 
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password) {
  Watchdog.reset();

  // turn on Fona
  Serial.println("Turning Fona On...");
  digitalWrite(FONA_KEY, 0);
  delay(2000);
  digitalWrite(FONA_KEY, 1);
  delay(1000);
  isFonaOn = true;

  Serial.println(F("Initializing FONA....(May take 3 seconds)"));
  
  fonaSS.begin(4800); // if you're using software serial
  
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    Serial.println(F("Couldn't find FONA"));
    return false;
  }
  fonaSS.println("AT+CMEE=2");
  Serial.println(F("FONA is OK"));
  Watchdog.reset();
  Serial.println(F("Checking for network..."));
  while (fona.getNetworkStatus() != 1) {
   delay(500);
  }

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
  
  fona.setGPRSNetworkSettings(apn, username, password);

  Serial.println(F("Disabling GPRS"));
  fona.enableGPRS(false);
  
  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();

  Serial.println(F("Enabling GPRS"));
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn GPRS on"));  
    return false;
  }
  Watchdog.reset();

  return true;
}

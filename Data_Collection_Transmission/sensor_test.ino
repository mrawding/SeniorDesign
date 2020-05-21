#include <Adafruit_ADS1015.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_EC.h"
#include <EEPROM.h>
#include <Wire.h>
#include "MS5837.h"

#define ONE_WIRE_BUS 14
Adafruit_ADS1115 ads1115(0x48);  // construct an ads1115 at address 0x49
float Voltage = 0.0;

DFRobot_EC ec;
MS5837 pressure_sensor;

/*DS18B20 Temperature Sensor Details */
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temp_probe(&oneWire);

void setup(void)
{
  Serial.begin(9600);
  ads1115.begin();
  ads1115.setGain(GAIN_ONE);

  Wire.begin();

  temp_probe.begin();

  while (!pressure_sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }
  //ec.begin();
}

float adc0, adc1, adc2, adc3, temp;
float tdsValue = 0;
float ecValue = 0;

void loop(void)
{
  adc0 = ((float) ads1115.readADC_SingleEnded(0));
  adc1 = ((float) ads1115.readADC_SingleEnded(1));
  adc2 = ((float) ads1115.readADC_SingleEnded(2));
  adc3 = ((float) ads1115.readADC_SingleEnded(3));
  float temperature = read_temp();
  float pressure = (float) pressure_sensor.pressure();

/*
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVolatge = adc1 / compensationCoefficient; //temperature compensation
  tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value

  ecValue = ec.readEC(adc2, temperature);
*/

  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Dissolved Oxygen: "); Serial.println(adc0);
  Serial.print("Total Dissolved Solids: "); Serial.println(adc1);
  Serial.print("Conductivity: "); Serial.println(adc2);
  Serial.print("ISFET: "); Serial.println(adc3);
  Serial.print("Pressure: "); Serial.println(pressure);
  Serial.println(" ");

  delay(5000);
}

float read_temp() {
  temp_probe.requestTemperatures(); // Send the command to get temperatures

  float tempC = temp_probe.getTempCByIndex(0);

  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    return tempC;
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    return 0;
  }

}

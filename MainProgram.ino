#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "DHT.h"
// bla bla bunch of libraries bla bla

#define DHTPIN 4 // defining DHT sensors
#define DHTTYPE DHT22 // defining DHT sensors
#define DHTPIN2 5 // defining DHT sensors
#define DHTTYPE2 DHT22 // defining DHT sensors

Adafruit_ADS1015 ads1015; // defining ADC

//  ANALOG PORTS: 
//  1.  LUX
//  2.  SOIL MOISTURE
//  3.
//  4.

ESP8266WiFiMulti WiFiMulti;

// IMPORTANT DATA:    
    String addressT = "http://hellmann.cc/designDev/firebaseTestT.php?arduino_data=";   // }
    String addressH = "http://hellmann.cc/designDev/firebaseTestH.php?arduino_data=";   // } Addresses for the databases. 
    String addressTO = "http://hellmann.cc/designDev/firebaseTestTO.php?arduino_data="; // } will be outsourced to .config on SD in further iterations.
    String addressHO = "http://hellmann.cc/designDev/firebaseTestHO.php?arduino_data="; // }
    String finalAddress = String("");
    float temp[4]; 
    float humid[4]; // } arrays for data. 
    float lux[4];
    int locationsConnected = 1; // amount of additional locations, will be automatically detected and written to config on startup in further iterations.
    int typeOfSensor[4] = {1, 2, 0, 0}; //  defining the types of sensors attached to ports 1-4, this will be auto detected in further iterations. 
    char sType; // random var
    bool success = false; // random var
    DHT dht(DHTPIN, DHTTYPE); // defining DHT sensors
    DHT dht2(DHTPIN2, DHTTYPE2); // defining DHT sensors
//---

void setup() 
{
    WiFiMulti.addAP("Ross Students","R0ssStud3nT"); //connecting to WIFI
    pinMode(LED_BUILTIN, OUTPUT); //enabling built in LED for debugging
    Serial.begin(115200); //starting serial for debugging, might be removed in later iterations
    dht.begin();  // initiate DHT22 temp / hum. sensor
    dht2.begin(); // ...
    ads1015.begin();  // initiate ADC
    locationsConnected += 1;  // adding 1 to correct for base station
}

float test = 21.69;

void loop() {

for(int count = 0; count < locationsConnected; count++)
{
  int sensor = typeOfSensor[count];
  // Main switch/case, checks different sensors here
  switch(sensor)
  {
    case 1: // 1 = base station, base station has ( at the moment ) only 1 DHT22 sensor.
      // no need to set MUX output, sensor 1 is on base station and always connected
      temp[0] = dht.readTemperature();
      humid[0] = dht.readHumidity();
        finalAddress = assembleWebAddress(addressT, temp[0]);
        sendData(finalAddress); // using the sendData function below to update database value of temp.
        finalAddress = assembleWebAddress(addressH, humid[0]);
        sendData(finalAddress); // using the sendData function below to update database value of humid.
        finalAddress = String(""); // clearing finalAdress for future reuse
      delay(1000); //sleep for 1 seconds to allow THING-ESP8266 to execute, not doing this might lead to crashes and wifi dc's
    break;

    case 2: // 2 = remote sensor 1, currently consisting of 1 DHT22, 4 analog sensors coming soon(tm)
      // next iteration of code: MUX output will be set to #00 here, thus connecting the sensor on port 1 of our MUX (=multiplexer)
      // will be doing function below for ChangeMUXOutput
      temp[1] = dht2.readTemperature();
      humid[1] = dht2.readHumidity();
      // <read other sensors>

      // </read other sensors>
      finalAddress = assembleWebAddress(addressTO, temp[1]);
      sendData(finalAddress); // using the sendData function below to update database value of temp.
      finalAddress = assembleWebAddress(addressHO, humid[1]);
      sendData(finalAddress); // using the sendData function below to update database value of humid.
      // <send other data>

      // </send other data>
      finalAddress = String(""); // clearing finalAdress for future reuse
      delay(1000); //sleep for 1 seconds to allow THING-ESP8266 to execute, not doing this might lead to crashes and wifi dc's
    break;
  } // closing switch
} // closing for

ESP.deepSleep(300000000); // Deep Sleep for 5 minutes, saves power ( in final version, power LED will be desoldered to save even more energy
}

void blink(int x) // simple function to blink the builtin led for a certain amount of time, used for debugging
{
  for(int y = 0; y <= x; y++)
  {
    digitalWrite(LED_BUILTIN, LOW); //low = on (weird ikr)
    delay(1000); 
    digitalWrite(LED_BUILTIN, HIGH); //...
    delay(1000);     
  }
}

String assembleWebAddress(String addrInput, float tempInput) // assemlbing the input string and the input float to address ( wont work on LUX, will have to write one for non-floats )
{ 
  char temporary[5]; // temporary char array
  dtostrf(tempInput, 5, 2, temporary); // using C's dtostrf to convert the float to char array ( 5 charactes including comma, Max. 2 behong comma )
  String tempStorage = String(temporary); // using Arduinos string constructor to convert fixed size char array to fluid string
  String addrOutput = String(addrInput + temporary); // adding both strings together to get final adderess
  return addrOutput;
}

void sendData(String addressInput) // connecting to .php on webserver to send data. 
{
  bool sendComp = false;
  while(sendComp = false)
  {
   if((WiFiMulti.run() == WL_CONNECTED))  // check if connected to wifi.
    {                
      HTTPClient http; // defining http client
      http.begin(addressInput);  // beginning http request to address
      int httpCode = http.GET(); // getting http output code to check for error
      if(httpCode < 0) // negative= some error
      {
          blink(2); // blink to indicate error
          break; // exiting enclosing if  to try again
      }
      else
      {
        sendComp = true;
        delay(1000); // delay to keep wifi up
      }
      http.end();
    }
  }
}

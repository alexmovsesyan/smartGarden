
#include "TimeLib.h"
#include <Wire.h>
#include <Arduino.h>
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <time.h>
#include <JsonListener.h>
#include "OpenWeatherMapForecast.h"
#include <string> 

OpenWeatherMapForecast client;

String OPEN_WEATHER_MAP_APP_ID = "040e7ee8ec64231e9042b581356054c3";
String OPEN_WEATHER_MAP_LOCATION = "Anaheim,US";
String OPEN_WEATHER_MAP_LANGUAGE = "en";
boolean IS_METRIC = false;
uint8_t MAX_FORECASTS = 4;
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
WiFiClient wifiClient;

char auth[] = "5522ea3119c6477197062f6ae5049beb";

const char* WIFI_SSID     = "UCInet Mobile Access";
const char* WIFI_PASSWORD = "";

int soilVal = 0; //value for storing moisture value 
const int soilPin = A0;//Declare a variable for the soil moisture sensor 
const int soilPower = 0;
const int tooDry = 900;
const int solenoidPin = 4;
const int timezone = 7;
const int dst = 0;
bool indoor = false;
bool needsWater = false;
int dayToWater; 
bool dayFound = false;

BLYNK_WRITE(V3) {
  long startTimeInSecs = param[0].asLong();
  Serial.println(startTimeInSecs);
  Serial.println();
}

void setup() {
  Serial.begin(9600); 
  connectWifi();
  
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);

  Wire.begin();
  
  if (zopt220xSetup() == false)
  {
    Serial.println("Sensor failed to respond. Check wiring.");
    while (1);
  }
  Serial.println("ZOPT220x online!");

  enableALSSensing(); //ALS + COMP channels activated
  
  setMeasurementRate(2); //100ms default
  setResolution(2); //18 bit, 100ms, default
  setGain(1); //Default for ALS
  
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSWORD);

  pinMode(solenoidPin, OUTPUT);

  configTime(-7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
 
  time_t now = time(nullptr);
  Blynk.virtualWrite(V4, "N/A");
  Blynk.virtualWrite(V5, "N/A");
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();

  time_t now = time(nullptr);
  if(needsWater==true && day(now)==dayToWater && hour(now)==8){
    water();
    needsWater = false;
  }
  
  Serial.print("Soil Moisture = ");    
  //get soil moisture value from the function below and print it
  int moisture = readSoil();
  Serial.println(moisture);
  
  if(moisture<= tooDry && dayFound == false){
    Serial.println("too dry");
    needsWater = true;
    dayToWater = predictWaterDay();
    String out = String(month(now));
    out+= "/";
    out+= String(dayToWater);
    Blynk.virtualWrite(V5, out);
  }

  Serial.print("Light = " + readLight());
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}

int readSoil()
{
    digitalWrite(soilPower, HIGH);//turn D7 "On"
    delay(10);//wait 10 milliseconds 
    soilVal = analogRead(soilPin);//Read the SIG value form sensor 
    Blynk.virtualWrite(V1, soilVal);
    digitalWrite(soilPower, LOW);//turn D7 "Off"
    return soilVal;//send current moisture value
}

long readLight(){
  long light = getALS();
  Blynk.virtualWrite(V2, light);
  return light;
}

float readUV(){
  float uvIndex = getUVIndex();
  long UV = getUVB();
  Blynk.virtualWrite(V3, uvIndex);
  return uvIndex;
}

void water(){
  digitalWrite(solenoidPin, HIGH);
  delay(300);
  digitalWrite(solenoidPin, LOW);  
  delay(300);
  digitalWrite(solenoidPin, HIGH);
  delay(300);
  digitalWrite(solenoidPin, LOW);  
  delay(2000);
  Blynk.virtualWrite(V4, getCurrentDate());
  Serial.println("watered!");
}


void getTime(){
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  Serial.print("Seconds :");
  Serial.println(second(now));
  Serial.print("Hour :");
  Serial.println(hour(now));
  Serial.print("Day :");
  Serial.println(day(now));
}

int predictWaterDay(){
  dayFound = true;
  time_t now = time(nullptr);
  if(indoor == true){
    return day(now);
  }
  else{
    Serial.println("Getting Forecasts: ");
    OpenWeatherMapForecastData data[MAX_FORECASTS];
    client.setMetric(IS_METRIC);
    client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
    uint8_t allowedHours[] = {9};
    client.setAllowedHours(allowedHours, 2);
    uint8_t foundForecasts = client.updateForecasts(data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION, MAX_FORECASTS);
    time_t time;
    int tempMin = 200;
    int waterDay;

    
    for (uint8_t i = 0; i < 2; i++) {
      Serial.printf("tempMax: %f\n", data[i].tempMax);
      if(data[i].tempMax < tempMin){
        tempMin = data[i].tempMax;
        waterDay = day(now)+i+1;
      }
        
    }
    Serial.print("Water Day: ");
    Serial.println(waterDay);
    return waterDay;
  }
}

String getCurrentDate(){
  time_t now = time(nullptr);
  String date;
  date+= String(month(now));
  date+="/";
  date+= String(day(now));
  return date;
}


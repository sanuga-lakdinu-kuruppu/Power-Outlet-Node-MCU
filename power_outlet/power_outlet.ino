#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "DHT.h"


#define WIFI_SSID "Test"          //Device wifi username
#define WIFI_PASSWORD "12345678"  // Device wifi password

#define FIREBASE_HOST "test-58844-default-rtdb.asia-southeast1.firebasedatabase.app"  // Firebase host
#define FIREBASE_AUTH "Y2lmJfXlEvdxX7fwF0O64arhWNI1RRc46b0wPlij"                      //Firebase Auth code

#define OUTPUT_STATUS_PIN D0

//Temperature Sensor Configuration
#define TEMPERATURE_TYPE DHT11
#define TEMPERATURE_PIN D1

DHT dhtObj(TEMPERATURE_PIN, TEMPERATURE_TYPE);

bool OUTPUT_STATUS = false;
unsigned long TIMEPREVIOURMILLS_TEMP = 0;

void setup() {

  Serial.begin(9600);

  //Pin Configuration
  pinMode(OUTPUT_STATUS_PIN, OUTPUT);
  pinMode(TEMPERATURE_PIN, INPUT);

  //Wifi Configuration
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("."));
    delay(300);
  }
  Serial.println();
  Serial.println(F("Connected."));
  Serial.println(WiFi.localIP());
  Serial.println();

  //Firebase Configuration
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.print(F("Firebase Connecting.."));

  while (Firebase.failed() == true) {
    Serial.print(F("."));
    delay(300);
  }
  Serial.println();
  Serial.println(F("Firebase Connected."));
  Serial.println();

  //initializing dht sensor
  dhtObj.begin();
}

void loop() {

  //getting output_status every time
  OUTPUT_STATUS = Firebase.getBool("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/OUTPUT_STATUS");

  //setting output
  if (OUTPUT_STATUS) {
    Serial.println("true");
    digitalWrite(OUTPUT_STATUS_PIN, HIGH);
  } else {
    Serial.println("false");
    digitalWrite(OUTPUT_STATUS_PIN, LOW);
  }

  //every 5 second read temperature data and set inside the database
  if (millis() - TIMEPREVIOURMILLS_TEMP > 5000 || TIMEPREVIOURMILLS_TEMP == 0) {
    TIMEPREVIOURMILLS_TEMP = millis();
    readTemperatureData();
    checkEnvironmentCondition();
  }
}

void readTemperatureData() {

  //read temperature sensor data
  float temp = dhtObj.readTemperature();
  float hum = dhtObj.readHumidity();

  //check read data is correct
  if (isnan(temp) || isnan(hum)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //set data inside the database
  Firebase.setFloat("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/TEMPERATURE/CURRENT", temp);
  Firebase.setFloat("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/HUMIDITY/CURRENT", hum);
}

void checkEnvironmentCondition() {

  //getting safe temperature value range
  float upperTemp = Firebase.getFloat("META_DATA/SAFE_TEMPERATURE/UPPER");
  float lowerTemp = Firebase.getFloat("META_DATA/SAFE_TEMPERATURE/LOWER");

  //getting safe humidity value range
  float upperHum = Firebase.getFloat("META_DATA/SAFE_HUMIDITY/UPPER");
  float lowerHum = Firebase.getFloat("META_DATA/SAFE_HUMIDITY/LOWER");

  //getting current temperature and humidity
  float humCheck = Firebase.getFloat("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/HUMIDITY/CURRENT");
  float tempCheck = Firebase.getFloat("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/TEMPERATURE/CURRENT");

  //check whether current values are inside safe the range
  if (upperTemp < tempCheck || upperHum < humCheck) {
    OUTPUT_STATUS = false;
  } else if (lowerTemp > tempCheck || lowerHum > humCheck) {
    OUTPUT_STATUS = false;
  }
  else{
    OUTPUT_STATUS = true;
  }

  //setting device status
  Firebase.setBool("HOUSES/HOUSES_1/ADAPTORS/ADAPTOR_1/OUTPUT_STATUS", OUTPUT_STATUS);
}

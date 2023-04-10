#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define WIFI_SSID "HUAWEI Y9 2019" //Device wifi username
#define WIFI_PASSWORD "SSss11.." // Device wifi password

#define FIREBASE_HOST "test-58844-default-rtdb.asia-southeast1.firebasedatabase.app" // Firebase host
#define FIREBASE_AUTH "Y2lmJfXlEvdxX7fwF0O64arhWNI1RRc46b0wPlij" //Firebase Auth code

#define pinDHT11INPUT D1
#define pinACOUTPUT D0
#define typeDHT11 DHT11

#define pinWIFISTATUSOUTPUT D6
#define pinFIREBASESTATUSOUTPUT D7
#define pinPOWEROUTPUTSTATUSOUTPUT D8
#define pinACOUTPUT D0

unsigned long timePrevMillis = 0;
bool deviceStatus = false;

//Temperature Sensor Configuration
DHT dhtObj(pinDHT11INPUT,typeDHT11);

void setup() {
 
 Serial.begin(9600);

  //Pin Configuration
  pinMode(pinDHT11INPUT,INPUT);
  pinMode(pinACOUTPUT,OUTPUT);
  pinMode(pinWIFISTATUSOUTPUT,OUTPUT);
  pinMode(pinFIREBASESTATUSOUTPUT,OUTPUT);
  pinMode(pinPOWEROUTPUTSTATUSOUTPUT,OUTPUT);

  //Wifi Configuration  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(pinWIFISTATUSOUTPUT,HIGH);
    Serial.print(F("."));
    delay(300);
  }
  digitalWrite(pinWIFISTATUSOUTPUT,LOW);
  Serial.println();
  Serial.println(F("Connected."));
  Serial.println(WiFi.localIP());
  Serial.println();  

  //Firebase Configuration
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.print(F("Firebase Connecting.."));

  while(Firebase.failed() == true){
    digitalWrite(pinFIREBASESTATUSOUTPUT,HIGH);
    Serial.print(F("."));
    delay(300);    
    
  }
  digitalWrite(pinFIREBASESTATUSOUTPUT,LOW);
  Serial.println();
  Serial.println(F("Firebase Connected."));
  Serial.println();  
      
  Firebase.setString("DEVICES/EIFHa45JS/DEVICE_STATUS","Online");
  
  dhtObj.begin();
}

void loop() {
  
  //Pass Sensor Data to Database
  if( (millis() - timePrevMillis > 2000 || timePrevMillis == 0)){
        timePrevMillis = millis();
        
        //read temperature sensor data
        float temp = dhtObj.readTemperature();
        float hum = dhtObj.readHumidity();

        //check read data is correct
        if(isnan(temp) || isnan(hum)){
          Serial.println(F("Failed to read from DHT sensor!"));          
          return;
        }

          Firebase.setFloat("DEVICES/EIFHa45JS/TEMPERATURE",temp);
          Firebase.setFloat("DEVICES/EIFHa45JS/HUMIDITY",hum);
        
          Serial.print(F("Temperature Sensor data is stored.."));
          Serial.print(F("Temp : "));  
          Serial.print(temp);
          Serial.print(F("Hum : "));  
          Serial.println(hum);
          Serial.print(F("Path : "));
          Serial.println(F("DEVICES/EIFHa45JS/\n"));                    
   }

}

bool checkDeviceStatus(){
  if(Firebase.getString("DEVICES/EIFHa45JS/DEVICE_STATUS") == "Online"){
    return true;
  }  
  else{
    return false;
  }
}

bool checkDeviceOkay(){
  if(Firebase.getBool("DEVICES/EIFHa45JS/IS_OKAY") == true){
    return true;
  }  
  else{
    return false;
  }  
}

bool checkDevicePowerOutputStatus(){
  if(Firebase.getBool("DEVICES/EIFHa45JS/POWER_OUTPUT_STATUS") == true){
    return true;
  }  
  else{
    return false;
  }   
}

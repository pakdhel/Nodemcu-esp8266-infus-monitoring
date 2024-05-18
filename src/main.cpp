#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "FADHEL HAYAT"
#define WIFI_PASSWORD "manggarupi000"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBHhSeUw4OytlaBiPemZ7FTC5jJ_oO5sPw"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp866-monitoring-infus-default-rtdb.asia-southeast1.firebasedatabase.app" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// variable drip_rate
unsigned long lastDropTime = 0;
unsigned long totalDropTime = 0;
unsigned long totalDropTimeDisplay = 0;
unsigned long prevMillis = 0;
unsigned long interval = 3000;

float dripRate = 0;
float dropCountDisplay = 0;
int pin = 4;
int dropCount = 0;

void setup() {
  pinMode(pin, INPUT);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(200);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  unsigned long currMillis = millis();
  // debounceDelay = 2000, untuk kecepatan tetesan ~ 20, debounceDelay = 1000 untuk kecepatan tetesan ~ 30, debounceDelay = 900 untuk kecepatan tetesan ~ 20 - 60
  // deounceDelay = 900 untuk kecepatan tetesan ~ 35
  if (digitalRead(pin) == 1 && currMillis - lastDropTime > 900) { 
    unsigned long dropTime = millis();
    unsigned long diff = dropTime - lastDropTime;

    totalDropTimeDisplay += diff;
    dropCountDisplay++;

    totalDropTime += diff;
    dropCount++;
    lastDropTime = dropTime;
  }

  if (currMillis - prevMillis >= interval) { // interval = 3000 untuk kecepatan tetesan ~ 20 - 60
    if (totalDropTime != 0) {
      dripRate = (float)dropCount / ((float)totalDropTime / 60000.0);  
    } else {
      dripRate = 0;
    }

    // Reset totalDropTime, dropCount, dan prevMillis
    totalDropTime = 0;
    dropCount = 0;
    prevMillis = currMillis;

  }

  Serial.print("total waktu tiap tetesan: ");
  Serial.print((float)totalDropTimeDisplay/1000);
  Serial.print(", drop: ");
  Serial.print(dropCountDisplay);
  Serial.print(", drip rate: ");
  Serial.println(dripRate);

  if (Firebase.ready() && signupOK){
    if (Firebase.RTDB.setFloat(&fbdo, "pasien/1/drip_rate", dripRate)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.print("Drip rate: ");
      Serial.println(dripRate);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    } 
  }

}

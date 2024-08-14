#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "password"

// Insert Firebase project API Key
#define API_KEY ""

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "" 

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
int dropCountDisplay = 0;
int pin = 4;
int dropCount = 0;
const float tetesPerLiter = 13;
bool initialWeightRetrieved = false;
float initialInfusionWeight = 0.0;

float lastRate = 0;

float infusionVolume(int dropCountDisplay);
float smoothDripRate(float currentRate);

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

  config.token_status_callback = tokenStatusCallback; 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  unsigned long currMillis = millis();

  // dynamic debounceDelay based on drip rate
  unsigned long debounceDelay = 100;
  if (dripRate > 0) {
    debounceDelay = 30000 / dripRate / 2;
  }

  static unsigned long debounceTime = 0;
  bool lastState = false;
  bool dropDetected = false;

  int currentState = digitalRead(pin);
  if (currentState == 1 && lastState == 0 && (currMillis - debounceTime) > debounceDelay) { 
    debounceTime = currMillis;
    dropDetected = true;
  }
  lastState = currentState;

  if (dropDetected) {
    unsigned long dropTime = millis();
    unsigned long diff = dropTime - lastDropTime;
    lastDropTime = dropTime;
    totalDropTime += diff;
    dropCount++;
    
    dropCountDisplay++;
    totalDropTimeDisplay += diff;
  }


  if (currMillis - prevMillis >= interval) {
    if (totalDropTime != 0 && dropCount != 0) {
      dripRate = (float)dropCount / ((float)totalDropTime / 60000.0);  
    } else {
      dripRate = 0;
    }

    dripRate = smoothDripRate(dripRate);

    // Reset totalDropTime, dropCount, dan prevMillis
    totalDropTime = 0;
    dropCount = 0;
    prevMillis = currMillis;

    if (!initialWeightRetrieved) {
      if (Firebase.RTDB.getFloat(&fbdo, "pasien/1/initial_infusion_weight")) {
        initialInfusionWeight = fbdo.floatData();
        Serial.print("initialInfusionWeight: ");
        Serial.println(initialInfusionWeight);
        initialWeightRetrieved = true;
      } else {
        Serial.println(fbdo.errorReason());
      }
    }

    float volDigunakan = infusionVolume(dropCountDisplay);
    float remainInfusionWeight = initialInfusionWeight - volDigunakan;

    Serial.print("total waktu tiap tetesan: ");
    Serial.print((float)totalDropTimeDisplay/1000);
    Serial.print(", drop: ");
    Serial.print(dropCountDisplay);
    Serial.print(", drip rate: ");
    Serial.println(dripRate);

    if (Firebase.ready() && signupOK){
      Firebase.RTDB.setFloat(&fbdo, "pasien/1/drip_rate", dripRate);
      Firebase.RTDB.setDouble(&fbdo, "pasien/1/time", millis());
      if (Firebase.RTDB.setFloat(&fbdo, "pasien/1/infusion_weight", remainInfusionWeight)) {
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
        Serial.print("remainInfusionWeight: ");
        Serial.println(remainInfusionWeight);
      } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
  }
}

float infusionVolume(int dropCountDisplay) {
  return (float) dropCountDisplay / tetesPerLiter; 
}

float smoothDripRate(float currentRate) {
  float smoothedRate = (lastRate * 0.7 ) + (currentRate * 0.3);
  lastRate = smoothedRate;
  return smoothedRate;
}

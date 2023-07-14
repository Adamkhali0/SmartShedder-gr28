#include <Wire.h>
#include <Adafruit_INA219.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP32)
  #include <ESP32WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Insert your network credentials
#define WIFI_SSID "Adam"
#define WIFI_PASSWORD "adam1234"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDUuM5T1MZ8oXIQZhiy9KeuJth4cXCuT6I"

// Insert RTDB URL
#define DATABASE_URL "https://iotgrp28-default-rtdb.europe-west1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
float current_mA;
float voltage_V;
float power_mW;
bool signupOK = false;

Adafruit_INA219 ina219;

const int vanControlPin = 15;  // Broche de contrôle du van
bool vanStatus = false;  // État initial du van (éteint)

// LCD I2C
hd44780_I2Cexp lcd;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, LOW);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("Measuring current, voltage, and power with INA219...");
}

void loop() {
  float shuntvoltage = 0;
  float busvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.setFloat(&fbdo, "/mesures/courant", current_mA)) {
      Serial.println("Current sent to Firebase: " + String(current_mA));
    }
    else {
      Serial.println("Failed to send current to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/mesures/tension", busvoltage)) {
      Serial.println("Voltage sent to Firebase: " + String(busvoltage));
    }
    else {
      Serial.println("Failed to send voltage to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/mesures/puissance", power_mW)) {
      Serial.println("Power sent to Firebase: " + String(power_mW));
    }
    else {
      Serial.println("Failed to send power to Firebase. Reason: " + fbdo.errorReason());
    }

    if (power_mW < 500 && !vanStatus) {
      // Allumer le van
      digitalWrite(vanControlPin, HIGH);
      vanStatus = true;  // Mettre à jour l'état du van (allumé)
      Serial.println("Van turned ON");
    }
    else if (power_mW >= 500 && vanStatus) {
      // Éteindre le van
      digitalWrite(vanControlPin, LOW);
      vanStatus = false;  // Mettre à jour l'état du van (éteint)
      Serial.println("Van turned OFF");
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("I  : " + String(current_mA) + " mA");
  lcd.setCursor(0, 1);
  lcd.print("P  : " + String(power_mW) + " mW");

  delay(2000);
}

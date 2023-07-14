#include <Wire.h>
#include <Adafruit_INA219.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ESPAsyncWebServer.h>
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

AsyncWebServer server(80);

const int van1ControlPin = 14;
const int van2ControlPin2 = 15;  // Broche de contrôle des vans

bool van1Status = false;  // État initial du van 1 (éteint)
bool van2Status = false;  // État initial du van 2 (éteint)


// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
float current_mA;
float voltage_V;
float power_mW;
float current_mA_2; // Mesure du deuxième INA219
float voltage_V_2; // Mesure du deuxième INA219
float power_mW_2; // Mesure du deuxième INA219
bool signupOK = false;

Adafruit_INA219 ina219;
Adafruit_INA219 ina219_2(0x41); // Deuxième INA219

const int vanControlPin = 15;
const int vanControlPin2 = 14;  // Broche de contrôle du van
bool vanStatus = false;  // État initial du van (éteint)

// LCD I2C
hd44780_I2Cexp lcd;

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  lcd.begin(20, 4);  // Initialiser l'écran LCD 2004
  lcd.print("Bonjour M.KHALI  :)");
  delay(2000);
  lcd.clear();

  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, HIGH);

  pinMode(vanControlPin2, OUTPUT);
  digitalWrite(vanControlPin2, HIGH);

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

  /* Assign the API key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign-up successful");
    signupOK = true;
  }
  else {
    Serial.printf("Sign-up failed. Reason: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  if (!ina219_2.begin()) {
    Serial.println("Failed to find second INA219 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("Measuring current, voltage, and power with INA219...");

  setupWiFi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  String html = "<html><head>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; }";
  html += "h1 { font-size: 24px; font-weight: bold; }";
  html += "h2 { font-size: 18px; font-weight: bold; }";
  html += "h3 { font-size: 16px; font-weight: bold; }";
  html += "p { font-size: 14px; }";
  html += "meter { width: 200px; }";
  html += ".green-gauge { --gauge-color: #4CAF50; }";
  html += ".yellow-gauge { --gauge-color: #FFEB3B; }";
  html += ".red-gauge { --gauge-color: #F44336; }";
  html += "button { padding: 8px 12px; font-size: 14px; }";
  html += "a { margin-right: 10px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Van control</h1>";
  
  html += "<h2>State of the Vans:</h2>";
  html += "<p>Van 1: ";
  html += van1Status ? "ON" : "OFF";
  html += "</p>";
  html += "<p>Van 2: ";
  html += van2Status ? "ON" : "OFF";
  html += "</p>";

  html += "<h3>Current 1: <meter min='0' max='1000' value='";
  html += String(ina219.getCurrent_mA(), 2);
  html += "' class='";
  html += ina219.getCurrent_mA() < 500 ? "green-gauge" : (ina219.getCurrent_mA() < 800 ? "yellow-gauge" : "red-gauge");
  html += "'></meter> ";
  html += String(ina219.getCurrent_mA(), 2);
  html += " mA</h3>";
  
  html += "<h3>Current 2: <meter min='0' max='1000' value='";
  html += String(ina219_2.getCurrent_mA(), 2);
  html += "' class='";
  html += ina219_2.getCurrent_mA() < 500 ? "green-gauge" : (ina219_2.getCurrent_mA() < 800 ? "yellow-gauge" : "red-gauge");
  html += "'></meter> ";
  html += String(ina219_2.getCurrent_mA(), 2);
  html += " mA</h3>";
  
  html += "<h3>Power 1: <meter min='0' max='500' value='";
  html += String(ina219.getPower_mW(), 2);
  html += "' class='";
  html += ina219.getPower_mW() < 200 ? "green-gauge" : (ina219.getPower_mW() < 400 ? "yellow-gauge" : "red-gauge");
  html += "'></meter> ";
  html += String(ina219.getPower_mW(), 2);
  html += " mW</h3>";
  
  html += "<h3>Power 2: <meter min='0' max='500' value='";
  html += String(ina219_2.getPower_mW(), 2);
  html += "' class='";
  html += ina219_2.getPower_mW() < 200 ? "green-gauge" : (ina219_2.getPower_mW() < 400 ? "yellow-gauge" : "red-gauge");
  html += "'></meter> ";
  html += String(ina219_2.getPower_mW(), 2);
  html += " mW</h3>";
  float power_total = power_mW + power_mW_2;
  html += "<h3>Total power: <meter min='0' max='1000' value='";
  html += String(ina219.getPower_mW() + ina219_2.getPower_mW(), 2);
  html += "' class='";
  html += (ina219.getPower_mW() + ina219_2.getPower_mW()) < 700 ? "green-gauge" : ((ina219.getPower_mW() + ina219_2.getPower_mW()) < 900 ? "yellow-gauge" : "red-gauge");
  html += "'></meter> ";
  html += String(ina219.getPower_mW() + ina219_2.getPower_mW(), 2);
  html += " mW</h3>";
  
  html += "<a href=\"/on-van1\"><button>Turn ON Van 1</button></a>";
  html += "<a href=\"/off-van1\"><button>Turn OFF Van 1</button></a>";
  html += "<a href=\"/on-van2\"><button>Turn ON Van 2</button></a>";
  html += "<a href=\"/off-van2\"><button>Turn OFF Van 2</button></a>";
  html += "</body></html>";
  request->send(200, "text/html", html);
});

server.on("/on-van1", HTTP_GET, [](AsyncWebServerRequest *request){
  digitalWrite(van1ControlPin, HIGH);
  van1Status = true;
  request->redirect("/");
});

server.on("/off-van1", HTTP_GET, [](AsyncWebServerRequest *request){
  digitalWrite(van1ControlPin, LOW);
  van1Status = false;
  request->redirect("/");
});

server.on("/on-van2", HTTP_GET, [](AsyncWebServerRequest *request){
  digitalWrite(van2ControlPin2, HIGH);
  van2Status = true;
  request->redirect("/");
});

server.on("/off-van2", HTTP_GET, [](AsyncWebServerRequest *request){
  digitalWrite(van2ControlPin2, LOW);
  van2Status = false;
  request->redirect("/");
});

server.begin();
}

void loop() {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float shuntvoltage_2 = 0; // Mesure du deuxième INA219
  float busvoltage_2 = 0; // Mesure du deuxième INA219

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  shuntvoltage_2 = ina219_2.getShuntVoltage_mV(); // Mesure du deuxième INA219
  busvoltage_2 = ina219_2.getBusVoltage_V(); // Mesure du deuxième INA219
  current_mA_2 = ina219_2.getCurrent_mA(); // Mesure du deuxième INA219
  power_mW_2 = ina219_2.getPower_mW(); // Mesure du deuxième INA219

  float power_total = power_mW + power_mW_2;
  
  if (power_total < 800 && !vanStatus) {
    // Allumer les vans
    digitalWrite(van1ControlPin, HIGH);
    digitalWrite(van2ControlPin2, HIGH);
    vanStatus = true;  // Mettre à jour l'état des vans (allumés)
    van1Status = true;
    van2Status = true;
  } else if (power_total >= 800 && vanStatus) {
    // Éteindre les vans
    digitalWrite(van1ControlPin, LOW);
    digitalWrite(van2ControlPin2, HIGH);
    vanStatus = false;  // Mettre à jour l'état des vans (éteints)
    van1Status = false;
    van2Status = true;
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.setFloat(&fbdo, "/Van1/current", current_mA)) {
      Serial.println("Van 1 Current sent to Firebase: " + String(current_mA));
    }
    else {
      Serial.println("Failed to send Van 1 current to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/Van1/voltage", busvoltage)) {
      Serial.println("Van 1 Voltage sent to Firebase: " + String(busvoltage));
    }
    else {
      Serial.println("Failed to send Van 1 voltage to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/Van1/power", power_mW)) {
      Serial.println("Van 1 Power sent to Firebase: " + String(power_mW));
    }
    else {
      Serial.println("Failed to send Van 1 power to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/Van2/current", current_mA_2)) {
      Serial.println("Van 2 Current sent to Firebase: " + String(current_mA_2));
    }
    else {
      Serial.println("Failed to send Van 2 current to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/Van2/voltage", busvoltage_2)) {
      Serial.println("Van 2 Voltage sent to Firebase: " + String(busvoltage_2));
    }
    else {
      Serial.println("Failed to send Van 2 voltage to Firebase. Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "/Van2/power", power_mW_2)) {
      Serial.println("Van 2 Power sent to Firebase: " + String(power_mW_2));
    }
    else {
      Serial.println("Failed to send Van 2 power to Firebase. Reason: " + fbdo.errorReason());
    }
  }

   // Affichage des données
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Current 1: ");
  lcd.print(String(current_mA, 2));
  lcd.print(" mA");

  lcd.setCursor(0, 1);
  lcd.print("Current 2: ");
  lcd.print(String(current_mA_2, 2));
  lcd.print(" mA");

  lcd.setCursor(0, 2);
  lcd.print("Power 1: ");
  lcd.print(String(power_mW, 2));
  lcd.print(" mW");

  lcd.setCursor(0, 3);
  lcd.print("Power 2: ");
  lcd.print(String(power_mW_2, 2));
  lcd.print(" mW");

  // Attente avant d'effacer l'écran
  delay(3000);

  // Effacement de l'écran
  lcd.clear();

  // Affichage de la puissance totale
  lcd.setCursor(0, 0);
  lcd.print("Total Power: ");
  lcd.setCursor(6, 2);
  lcd.print(String(power_total, 2));
  lcd.print(" mW");

  delay(2000);
}

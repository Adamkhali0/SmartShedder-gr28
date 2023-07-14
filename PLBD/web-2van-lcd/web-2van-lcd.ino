#include <Wire.h>
#include <Adafruit_INA219.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

Adafruit_INA219 ina219;
Adafruit_INA219 ina219_2(0x41);  // Adresse du deuxième INA219

hd44780_I2Cexp lcd;

bool vanStatus = false;  // État initial des vans (éteints)
bool van1Status = false;  // État initial du van 1 (éteint)
bool van2Status = false;  // État initial du van 2 (éteint)

const int van1ControlPin = 14;
const int van2ControlPin = 15;  // Broche de contrôle des vans

const char* ssid = "Adam";
const char* password = "adam1234";

AsyncWebServer server(80);

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) 
{
  lcd.begin(20, 4);  // Utilisez 20 colonnes et 4 lignes pour l'écran LCD 2004
  lcd.print("Bonjour M.KHALI");
  delay(2000);
  lcd.clear();

  pinMode(van1ControlPin, OUTPUT);
  pinMode(van2ControlPin, OUTPUT);
  digitalWrite(van1ControlPin, LOW);
  digitalWrite(van2ControlPin, LOW);
  
  delay(5000);
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  lcd.begin(20, 4);  // Initialiser l'écran LCD 2004
  lcd.print("Bonjour M.KHALI  :)");
  delay(2000);
  lcd.clear();
  
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  if (!ina219_2.begin()) {
    Serial.println("Failed to find second INA219 chip");
    while (1) { delay(10); }
  }

  Serial.println("Measuring voltage and current with INA219 ...");

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
  digitalWrite(van2ControlPin, HIGH);
  van2Status = true;
  request->redirect("/");
});

server.on("/off-van2", HTTP_GET, [](AsyncWebServerRequest *request){
  digitalWrite(van2ControlPin, LOW);
  van2Status = false;
  request->redirect("/");
});

server.begin();

}

void loop(void) 
{
  float current_mA = 0;
  float power_mW = 0;
  float current_mA_2 = 0;
  float power_mW_2 = 0;

  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  current_mA_2 = ina219_2.getCurrent_mA();
  power_mW_2 = ina219_2.getPower_mW();

  float power_total = power_mW + power_mW_2;
  
  if (power_total < 1100 && !vanStatus) {
    // Allumer les vans
    digitalWrite(van1ControlPin, HIGH);
    digitalWrite(van2ControlPin, HIGH);
    vanStatus = true;  // Mettre à jour l'état des vans (allumés)
    van1Status = true;
    van2Status = true;
  } else if (power_total >= 1100 && vanStatus) {
    // Éteindre les vans
    digitalWrite(van1ControlPin, LOW);
    digitalWrite(van2ControlPin, LOW);
    vanStatus = false;  // Mettre à jour l'état des vans (éteints)
    van1Status = false;
    van2Status = false;
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

  // Attente avant le prochain affichage
  delay(2000);
}

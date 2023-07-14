#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <WiFi.h>
#include <Adafruit_INA219.h>

// Remplacez avec vos identifiants réseau
const char* ssid = "Adam";
const char* password = "adam1234";

// Définition du numéro de port du serveur web
WiFiServer server(80);

// Variables pour stocker la requête HTTP
String header;
String etatSortie26 = "éteint";
String etatSortie27 = "éteint";
const int brocheSortie26 = 15;
const int brocheSortie27 = 27;

Adafruit_INA219 ina219;
hd44780_I2Cexp lcd;

bool etatVan = false;
const int brocheCommandeVan = 14;

void setup() {
  pinMode(brocheSortie26, OUTPUT);
  pinMode(brocheSortie27, OUTPUT);
  digitalWrite(brocheSortie26, LOW);
  digitalWrite(brocheSortie27, LOW);

  pinMode(brocheCommandeVan, OUTPUT);
  digitalWrite(brocheCommandeVan, LOW);

  delay(5000);
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  if (!ina219.begin()) {
    Serial.println("Échec de la détection de la puce INA219");
    while (1) {
      delay(10);
    }
  }

  lcd.begin(20, 4);  // Utilisez 20 colonnes et 4 lignes pour l'écran LCD 2004
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  Serial.println("Mesure de la tension et du courant avec INA219 ...");

  lcd.setBacklight(255); // Réglage de la luminosité du rétroéclairage LCD (valeurs possibles : 0-255)

  Serial.print("Connexion à ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connecté.");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /26/on") >= 0) {
              digitalWrite(brocheSortie26, HIGH);
              etatSortie26 = "allumé";
            } else if (header.indexOf("GET /26/off") >= 0) {
              digitalWrite(brocheSortie26, LOW);
              etatSortie26 = "éteint";
            } else if (header.indexOf("GET /27/on") >= 0) {
              digitalWrite(brocheSortie27, HIGH);
              etatSortie27 = "allumé";
            } else if (header.indexOf("GET /27/off") >= 0) {
              digitalWrite(brocheSortie27, LOW);
              etatSortie27 = "éteint";
            }

            client.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            client.println("<body><h1>Serveur Web ESP32</h1>");
            client.println("<p>GPIO 26 - État " + etatSortie26 + "</p>");
            client.println("<p><a href=\"/26/on\"><button class=\"button\">Allumer</button></a></p>");
            client.println("<p><a href=\"/26/off\"><button class=\"button button2\">Éteindre</button></a></p>");
            client.println("<p>GPIO 27 - État " + etatSortie27 + "</p>");
            client.println("<p><a href=\"/27/on\"><button class=\"button\">Allumer</button></a></p>");
            client.println("<p><a href=\"/27/off\"><button class=\"button button2\">Éteindre</button></a></p>");

            client.println("<h2>Tension et Courant</h2>");
            client.print("Tension d'alimentation:   ");
            client.print(ina219.getBusVoltage_V(), 2);
            client.println(" V<br>");
            client.print("Tension de détection: ");
            client.print(ina219.getShuntVoltage_mV(), 2);
            client.println(" mV<br>");
            client.print("Tension de charge:  ");
            client.print(ina219.getBusVoltage_V() + (ina219.getShuntVoltage_mV() / 1000), 2);
            client.println(" V<br>");
            client.print("Courant:       ");
            client.print(ina219.getCurrent_mA(), 2);
            client.println(" mA<br>");
            client.print("Puissance:         ");
            client.print(ina219.getPower_mW(), 2);
            client.println(" mW<br>");

            client.println("</body></html>");

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
  }

  float tensionDetection = 0;
  float tensionAlimentation = 0;
  float courant_mA = 0;
  float tensionCharge = 0;
  float puissance_mW = 0;

  tensionDetection = ina219.getShuntVoltage_mV();
  tensionAlimentation = ina219.getBusVoltage_V();
  courant_mA = ina219.getCurrent_mA();
  puissance_mW = ina219.getPower_mW();
  tensionCharge = tensionAlimentation + (tensionDetection / 1000);

  if (puissance_mW < 500 && !etatVan) {
    digitalWrite(brocheCommandeVan, HIGH);
    etatVan = true;
  } else if (puissance_mW >= 500 && etatVan) {
    digitalWrite(brocheCommandeVan, LOW);
    etatVan = false;
  }

  Serial.print("Tension de charge:  ");
  Serial.print(tensionCharge, 2);
  Serial.println(" V");
  Serial.print("Courant:       ");
  Serial.print(courant_mA, 2);
  Serial.println(" mA");
  Serial.print("Puissance:         ");
  Serial.print(puissance_mW, 2);
  Serial.println(" mW");
  Serial.println("");
  

  lcd.setCursor(9, 0);
  lcd.print(courant_mA, 2);
  lcd.print(" mA       ");
  
  lcd.setCursor(9, 1);
  lcd.print(tensionAlimentation, 2);
  lcd.print(" V        ");
  
  lcd.setCursor(9, 2);
  lcd.print(puissance_mW, 2);
  lcd.print(" mW       ");

  lcd.setCursor(0, 0);
  lcd.print("Current: " + String(courant_mA) + " mA");
  lcd.setCursor(0, 1);
  lcd.print("Voltage: " + String(tensionAlimentation) + " V");
  lcd.setCursor(0, 2);
  lcd.print("Power  : " + String(puissance_mW) + " mW");

  delay(2000);
}

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <BlynkSimpleEsp32.h>

// Blynk Auth Token
char auth[] = "CGByGKJrXStrORZl0P9404vMlhSvupvD";

// WiFi credentials
char ssid[] = "Adam";
char pass[] = "adam1234";

// INA219
Adafruit_INA219 ina219;

// LCD I2C
hd44780_I2Cexp lcd;

// Energy variables
float current_mA;
float power_mW;
float voltage_V;

bool vanStatus = false;  // État initial du van (éteint)
const int vanControlPin = 15;  // Broche de contrôle du van

void setup() {
  lcd.begin(20, 4);  // Utilisez 20 colonnes et 4 lignes pour l'écran LCD 2004
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, LOW);

  Serial.begin(115200);

  // Connect to WiFi
  Blynk.begin(auth, ssid, pass);

  // Initialize INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("Measuring current, voltage, and power with INA219...");
}

void loop() {
  // Read current, voltage, and power values from INA219
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  voltage_V = ina219.getBusVoltage_V();

  // Send current and power values to Blynk app
  Blynk.virtualWrite(V0, current_mA);
  Blynk.virtualWrite(V1, power_mW);
  Blynk.virtualWrite(V2, voltage_V);
  
  // Display current, voltage, and power values on LCD
  lcd.setCursor(0, 0);
  lcd.print("Current: " + String(current_mA) + " mA");
  lcd.setCursor(0, 1);
  lcd.print("Voltage: " + String(voltage_V) + " V");
  lcd.setCursor(0, 2);
  lcd.print("Power  : " + String(power_mW) + " mW");

  // Control the van based on power value
  if (power_mW < 450 && !vanStatus) {
    digitalWrite(vanControlPin, HIGH);  // Allumer le van
    vanStatus = true;  // Mettre à jour l'état du van (allumé)
  } else if (power_mW >= 450 && vanStatus) {
    digitalWrite(vanControlPin, LOW);  // Éteindre le van
    vanStatus = false;  // Mettre à jour l'état du van (éteint)
  }

  Blynk.run();
  delay(2000);
}

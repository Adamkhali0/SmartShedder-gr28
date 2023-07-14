#include <Wire.h>
#include <Adafruit_INA219.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

Adafruit_INA219 ina219;
Adafruit_INA219 ina219_2(0x41);  // Adresse du deuxième INA219

hd44780_I2Cexp lcd;

bool vanStatus = false;  // État initial du van (éteint)
const int vanControlPin = 15;  // Broche de contrôle du van

void setup(void) 
{
  lcd.begin(20, 4);  // Utilisez 20 colonnes et 4 lignes pour l'écran LCD 2004
  lcd.print("Bonjour M.KHALI");
  delay(2000);
  lcd.clear();

  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, LOW);
  
  delay(5000);
  Serial.begin(115200);
  while (!Serial) {
      delay(1);
  }

  lcd.begin(20, 4);  // Initialiser l'écran LCD 2004
  
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  if (!ina219_2.begin()) {
    Serial.println("Failed to find second INA219 chip");
    while (1) { delay(10); }
  }

  Serial.println("Measuring voltage and current with INA219 ...");
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
  
  if (power_total < 1050 && !vanStatus) {
    // Allumer le van
    digitalWrite(vanControlPin, HIGH);
    vanStatus = true;  // Mettre à jour l'état du van (allumé)
  } else if (power_total >= 1050 && vanStatus) {
    // Éteindre le van
    digitalWrite(vanControlPin, LOW);
    vanStatus = false;  // Mettre à jour l'état du van (éteint)
  }
  
  // Affichage des données
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Current1: ");
  lcd.print(String(current_mA, 2));
  lcd.print(" mA");  // Afficher un seul chiffre après la virgule

  lcd.setCursor(0, 1);
  lcd.print("Current2: ");
  lcd.print(String(current_mA_2, 2));
  lcd.print(" mA");  // Afficher un seul chiffre après la virgule

  lcd.setCursor(0, 2);
  lcd.print("Power1  : ");
  lcd.print(String(power_mW, 2));
  lcd.print(" mW");  // Afficher sans chiffre après la virgule

  lcd.setCursor(0, 3);
  lcd.print("Power2  : ");
  lcd.print(String(power_mW_2, 2));
  lcd.print(" mW");  // Afficher sans chiffre après la virgule

  // Attente avant d'effacer l'écran
  delay(3000);

  // Effacement de l'écran
  lcd.clear();

  // Affichage de la puissance totale
  lcd.setCursor(0, 0);
  lcd.print("Total Power: ");
  lcd.setCursor(6, 2);
  lcd.print(String(power_total, 2));
  lcd.print(" mW");  // Afficher sans chiffre après la virgule

  // Attente avant le prochain affichage
  delay(2000);
}

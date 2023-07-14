#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

bool vanStatus = false;  // État initial du van (éteint)
const int vanControlPin = 15;  // Broche de contrôle du van

void setup(void) 
{
  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, LOW);
  
  delay(5000);
  Serial.begin(115200);
  while (!Serial) {
      delay(1);
  }

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  Serial.println("Measuring voltage and current with INA219 ...");
}

void loop(void) 
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  if (power_mW < 450 && !vanStatus) {
    // Allumer le van
    digitalWrite(vanControlPin, HIGH);
    vanStatus = true;  // Mettre à jour l'état du van (allumé)
  } else if (power_mW >= 450 && vanStatus) {
    // Éteindre le van
    digitalWrite(vanControlPin, LOW);
    vanStatus = false;  // Mettre à jour l'état du van (éteint)
  }
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  delay(2000);
}

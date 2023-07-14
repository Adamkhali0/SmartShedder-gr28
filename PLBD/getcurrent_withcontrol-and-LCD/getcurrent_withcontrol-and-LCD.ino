#include <Wire.h>
#include <Adafruit_INA219.h>
#include <LiquidCrystal_I2C.h>

Adafruit_INA219 ina219;
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool vanStatus = false;
const int vanControlPin = 15;

void setup(void)
{
  pinMode(vanControlPin, OUTPUT);
  digitalWrite(vanControlPin, LOW);

  delay(5000);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  if (!ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    while (1)
    {
      delay(10);
    }
  }

  lcd.begin(16, 2);
 lcd.print("Current: ");
  lcd.setCursor(0, 1);
  lcd.print("Power  : ");

  Serial.println("Measuring voltage and current with INA219 ...");

  // Réglage de la luminosité de l'écran LCD (valeurs possibles : 0-255)
  lcd.setBacklight(255); // Réglez la valeur en fonction de vos besoins
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

  if (power_mW < 500 && !vanStatus)
  {
    digitalWrite(vanControlPin, HIGH);
    vanStatus = true;
  }
  else if (power_mW >= 500 && vanStatus)
  {
    digitalWrite(vanControlPin, LOW);
    vanStatus = false;
  }

  Serial.print("Bus Voltage:   ");
  Serial.print(busvoltage);
  Serial.println(" V");
  Serial.print("Shunt Voltage: ");
  Serial.print(shuntvoltage);
  Serial.println(" mV");
  Serial.print("Load Voltage:  ");
  Serial.print(loadvoltage);
  Serial.println(" V");
  Serial.print("Current:       ");
  Serial.print(current_mA);
  Serial.println(" mA");
  Serial.print("Power:         ");
  Serial.print(power_mW);
  Serial.println(" mW");
  Serial.println("");

  lcd.setCursor(9, 0);
  lcd.print(current_mA);
  lcd.setCursor(9, 1);
   lcd.print(power_mW);

  delay(2000);
}

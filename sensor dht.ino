#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// inisiasi alamat lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Setting DHT
#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  lcd.init();          
  lcd.backlight();     
  
  Serial.begin(9600);  
  dht.begin();         
}

void loop() {
  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();

  // Jika sensor gagal membaca
  if (isnan(suhu) || isnan(kelembaban)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sensor Error!");
    delay(2000);
    return;
  }

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Suhu: ");
  lcd.print(suhu,1);
  lcd.print(" C");

  lcd.setCursor(0,1);
  lcd.print("Hum : ");
  lcd.print(kelembaban,1);
  lcd.print(" %");

  delay(2000);
}

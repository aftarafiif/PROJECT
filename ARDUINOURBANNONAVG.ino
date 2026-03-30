
#include <Simpletimer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <math.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ========== SENSOR KECEPATAN (VSS) ==========
const int IRSensorPin = 2;
int inputState;
int lastInputState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 0.5;
double kkbanspd = 0.00039878;  // URBAN
long endTime;
long startTime;
int RPM = 0;
double trip = 0;
double jarak = 0;
float lnTime = 0.5;

// ========== SENSOR SUHU (THERMISTOR) ==========
const int ThermistorPin = A0;
int Vo;
float R1 = 9400;
float logR2, R2, T, Tf;
int Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

// ========== SENSOR RPM COIL ==========
const int sensorRPMCoil = 3;
int inputState2;
int lastInputState2 = LOW;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 0.5;
unsigned long endTime2;
unsigned long startTime2;
float lnTime2 = 0;
int RPMCoil = 0;

// ========== SENSOR AFR ==========
const int sensorPin = A1;
float afrVoltage = 0.0;
float AFR_etanol = 0.0;

// ========== SENSOR TPS ==========
const int tpsPin = A2;
const float voltMin = 0.45;
const float voltMax = 4.53;
float tpsVoltage = 0.0;
int ThrottleStatus = 0;

// ========== VARIABEL WAKTU ==========
long time3;
long time;
unsigned long packetCounter = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 300; // Kirim tiap 500ms

Simpletimer timer1;

void setup() {
  Serial.begin(9600);
  
  pinMode(IRSensorPin, INPUT);
  pinMode(ThermistorPin, INPUT);
  pinMode(sensorRPMCoil, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  endTime = 0;
  endTime2 = 0;
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("URBAN SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("NO PRESSURE");
  delay(2000);
  lcd.clear();
  
  timer1.register_callback(transfer);
  
  Serial.println("#URBAN_START#");
}

void loop() {
  timer1.run(sendInterval);
  
  // ========== BACA VSS ==========
  time = millis();
  int currentSwitchState = digitalRead(IRSensorPin);
  
  if (currentSwitchState != lastInputState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentSwitchState != inputState) {
      inputState = currentSwitchState;
      if (inputState == HIGH) {
        calculateRPM();
      }
    }
  }
  lastInputState = currentSwitchState;
  
  // ========== BACA RPM COIL ==========
  int currentSwitchState2 = digitalRead(sensorRPMCoil);
  
  if (currentSwitchState2 != lastInputState2) {
    lastDebounceTime2 = millis();
  }
  
  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    if (currentSwitchState2 != inputState2) {
      inputState2 = currentSwitchState2;
      if (inputState2 == HIGH) {
        hitungRPMCOil();
      }
    }
  }
  lastInputState2 = currentSwitchState2;
}

// ========== FUNGSI SENSOR SUHU ==========
void SUHU() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.1);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15;
}

// ========== FUNGSI SENSOR AFR ==========
void AFR() {
  int sensorValue = analogRead(sensorPin);
  afrVoltage = sensorValue * (5.0 / 1023.0);
  float AFR_bensin = 2.375 * afrVoltage + 7.3125;
  float lamda = AFR_bensin / 14.7;
  AFR_etanol = lamda * 9.12;
}

// ========== FUNGSI SENSOR TPS ==========
void TPS() {
  int tpsValue = analogRead(tpsPin);
  tpsVoltage = tpsValue * (5.5 / 930.0);
  ThrottleStatus = (tpsVoltage - voltMin) / (voltMax - voltMin) * 100;
  ThrottleStatus = constrain(ThrottleStatus, 0, 100);
}

// ========== FUNGSI HITUNG RPM VSS ==========
void calculateRPM() {
  startTime = lastDebounceTime;
  lnTime = startTime - endTime;
  
  // Cegah division by zero
  if ((startTime - endTime) > 0) {
    RPM = 60000 / (startTime - endTime);
  }
  endTime = startTime;
  trip++;
}

// ========== FUNGSI HITUNG RPM COIL ==========
void hitungRPMCOil() {
  startTime2 = lastDebounceTime2;
  lnTime2 = startTime2 - endTime2;
  
  if ((startTime2 - endTime2) > 0) {
    RPMCoil = 60000 / (startTime2 - endTime2);
  }
  endTime2 = startTime2;
}

// ========== FUNGSI TRANSFER DATA KE ESP ==========
void transfer() {
  time3 = millis() / 1000;
  int menit = time3 / 60;
  int detik = time3 % 60;
  
  // Hitung nilai
  int kecepatan = (int)((RPM * kkbanspd) * 60);
  int rpm_coil = RPMCoil * 2;
  
  // Baca semua sensor
  SUHU();
  AFR();
  TPS();
  
  // Buat JSON - SESUAI DENGAN FORMAT YANG DIHARAPKAN WEB!
  StaticJsonDocument<256> doc;
  
  doc["kecepatan"] = kecepatan;
  doc["rpm_coil"] = rpm_coil;
  doc["afr_etanol"] = round(AFR_etanol * 100) / 100.0;
  doc["throttle"] = ThrottleStatus;
  doc["suhu"] = (int)Tc;
  doc["packet"] = packetCounter++;
  
  JsonObject waktu = doc.createNestedObject("waktu");
  waktu["menit"] = menit;
  waktu["detik"] = detik;
  
  // Kirim ke ESP via Serial
  String output;
  serializeJson(doc, output);
  Serial.println(output);
  
  // Reset RPM
  RPM = 0;
  RPMCoil = 0;
  
  // UPDATE LCD
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print(kecepatan);
  
  lcd.setCursor(2,0);
  lcd.print("|");
  lcd.setCursor(2,1);
  lcd.print("|");

  lcd.setCursor(3, 1);
  lcd.print(Tc);
  lcd.setCursor(3,0);
  lcd.print(rpm_coil);

  lcd.setCursor(7,0);
  lcd.print("|");
  lcd.setCursor(7,1);
  lcd.print("|");
  
  lcd.setCursor(12,0);
  lcd.print("|");
  lcd.setCursor(6, 1);
  lcd.print(menit);
  lcd.print(":");
  if (detik < 10) lcd.print("0");
  lcd.print(detik);

  lcd.setCursor(8,0);
  lcd.print(ThrottleStatus);
  lcd.print(" %");
  lcd.setCursor(13,0);
  lcd.print(AFR_etanol, 1);  // Misa
  
  // Kedip LED
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
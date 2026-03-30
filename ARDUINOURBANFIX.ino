#include <Simpletimer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <math.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Ubah ke 20x4

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

// ========== AVERAGE FILTER UNTUK SPEED ==========
const int speedBufferSize = 4;
int speedBuffer[speedBufferSize];
int speedIndex = 0;
int speedAverage = 0;

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

// ========== VARIABEL TAMBAHAN ==========
long time3;
long time;
unsigned long packetCounter = 0;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 300; // Kirim tiap 300ms

Simpletimer timer1;

// Inisialisasi buffer speed
void initSpeedBuffer() {
  for (int i = 0; i < speedBufferSize; i++) {
    speedBuffer[i] = 0;
  }
}

// Fungsi untuk menghitung average speed
int calculateAverageSpeed(int newSpeed) {
  speedBuffer[speedIndex] = newSpeed;
  speedIndex = (speedIndex + 1) % speedBufferSize;
  
  long sum = 0;
  for (int i = 0; i < speedBufferSize; i++) {
    sum += speedBuffer[i];
  }
  return sum / speedBufferSize;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(IRSensorPin, INPUT);
  pinMode(ThermistorPin, INPUT);
  pinMode(sensorRPMCoil, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  endTime = 0;
  endTime2 = 0;
  
  // Inisialisasi buffer speed
  initSpeedBuffer();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 1);
  lcd.print("AGUS GANTENG");
  lcd.setCursor(6, 2);
  lcd.print("LOVEEEEE");
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

// ========== FUNGSI UPDATE LCD 20x4 DENGAN FORMAT TABEL ==========
void updateLCD(int kecepatan, int rpm_coil, float afr, int throttle, int suhu, int menit, int detik) {
  lcd.clear();
  
  // ===== BARIS 0 ===== (SPEED, RPM COIL, THROTTLE)
  // Format: SPD:xxxx|RPM:xxxxx|TPS:xxx%
  // Pos: 0-3: "SPD:"
  // Pos: 4-7: speed (4 digit)
  // Pos: 8: "|"
  // Pos: 9-12: "RPM:"
  // Pos: 13-17: rpm coil (5 digit)
  // Pos: 18: "|"
  // Pos: 19-? : "TPS:" (lanjut ke baris 1 karena tidak cukup)
  
  lcd.setCursor(0, 0);
  lcd.print("SPD:");
  
  // Tampilkan speed (4 digit dengan leading zero)
  lcd.setCursor(4, 0);
  if (kecepatan < 10) {
    lcd.print(" ");
    lcd.print(kecepatan);
  } else if (kecepatan < 100) {
    lcd.print(" ");
    lcd.print(kecepatan);
  } else if (kecepatan < 1000) {
    lcd.print(" ");
    lcd.print(kecepatan);
  } else {
    lcd.print(kecepatan);
  }
  
  lcd.setCursor(9, 0);
  lcd.print("|");
  
  lcd.setCursor(10, 0);
  lcd.print("RPM:");
  
  // Tampilkan rpm coil (5 digit dengan leading zero)
  lcd.setCursor(14, 0);
  if (rpm_coil < 10) {
    lcd.print(" ");
    lcd.print(rpm_coil);
  } else if (rpm_coil < 100) {
    lcd.print(" ");
    lcd.print(rpm_coil);
  } else if (rpm_coil < 1000) {
    lcd.print(" ");
    lcd.print(rpm_coil);
  } else if (rpm_coil < 10000) {
    lcd.print(" ");
    lcd.print(rpm_coil);
  } else {
    lcd.print(rpm_coil);
  }
  
  lcd.setCursor(18, 0);
  lcd.print(" ");
  
  // ===== BARIS 1 ===== (THROTTLE, SUHU, AFR)
  // Format: TPS:xxx%|TMP:xxxC|AFR:x.x
  // Pos: 0-4: "TPS:"
  // Pos: 5-7: throttle (3 digit) + "%"
  // Pos: 8: "|"
  // Pos: 9-13: "TMP:"
  // Pos: 14-16: suhu (3 digit) + "C"
  // Pos: 17: "|"
  // Pos: 18-19: "AFR" (lanjut)
  
  lcd.setCursor(0, 1);
  lcd.print("TPS:");
  
  // Tampilkan throttle (3 digit dengan leading zero)
  lcd.setCursor(4, 1);
  if (throttle < 10) {
    lcd.print(" ");
    lcd.print(throttle);
  } else if (throttle < 100) {
    lcd.print(" ");
    lcd.print(throttle);
  } else {
    lcd.print(throttle);
  }
  lcd.print("%");
  
  lcd.setCursor(9, 1);
  lcd.print("|");
  
  lcd.setCursor(10, 1);
  lcd.print("TMP:");
  
  // Tampilkan suhu (3 digit dengan leading zero)
  lcd.setCursor(14, 1);
  if (suhu < 10) {
    lcd.print(" ");
    lcd.print(suhu);
  } else if (suhu < 100) {
    lcd.print(" ");
    lcd.print(suhu);
  } else {
    lcd.print(suhu);
  }
  lcd.print(" ");
  
  lcd.setCursor(17, 1);
  lcd.print("");
  
  // ===== BARIS 2 ===== (AFR dan WAKTU)
  // Format: AFR:x.x|TIME:MM:SS
  // Pos: 0-4: "AFR:"
  // Pos: 5-8: afr (x.x)
  // Pos: 9: "|"
  // Pos: 10-15: "TIME:"
  // Pos: 16-20: MM:SS
  
  lcd.setCursor(0, 2);
  lcd.print("AFR:");
  
  // Tampilkan AFR (format x.x)
  lcd.setCursor(4, 2);
  int afrInt = (int)afr;
  int afrDec = (int)((afr - afrInt) * 10);
  if (afrInt < 10) lcd.print(" ");
  lcd.print(afrInt);
  lcd.print(".");
  lcd.print(afrDec);
  
  lcd.setCursor(9, 2);
  lcd.print("|");
  
  lcd.setCursor(10, 2);
  lcd.print("TIME:");
  
  // Tampilkan waktu (MM:SS)
  lcd.setCursor(15, 2);
  if (menit < 10) lcd.print("0");
  lcd.print(menit);
  lcd.print(":");
  if (detik < 10) lcd.print("0");
  lcd.print(detik);
  
  // ===== BARIS 3 ===== (PACKET COUNTER dan TRIP)
  // Format: PKT:xxxxx|TRIP:xxx.xx
  // Pos: 0-4: "PKT:"
  // Pos: 5-9: packet counter (5 digit)
  // Pos: 10: "|"
  // Pos: 11-16: "TRIP:"
  // Pos: 17-20: trip (km)
  
  lcd.setCursor(0, 3);
  lcd.print("PKT:");
  
  // Tampilkan packet counter (5 digit)
  lcd.setCursor(4, 3);
  unsigned long pkt = packetCounter;
  if (pkt < 10) {
    lcd.print(" ");
    lcd.print(pkt);
  } else if (pkt < 100) {
    lcd.print(" ");
    lcd.print(pkt);
  } else if (pkt < 1000) {
    lcd.print(" ");
    lcd.print(pkt);
  } else if (pkt < 10000) {
    lcd.print(" ");
    lcd.print(pkt);
  } else {
    lcd.print(pkt);
  }
  
  lcd.setCursor(9, 3);
  lcd.print("|");
  
  lcd.setCursor(10, 3);
  lcd.print("TRIP:");
  
  // Tampilkan trip (format xxx.xx km)
  // Catatan: trip masih perlu dihitung, untuk sementara pakai jarak
  float tripKm = trip * kkbanspd; // Konversi ke km
  lcd.setCursor(15, 3);
  int tripInt = (int)tripKm;
  int tripDec = (int)((tripKm - tripInt) * 100);
  if (tripInt < 10) {
    lcd.print(" ");
    lcd.print(tripInt);
  } else if (tripInt < 100) {
    lcd.print(tripInt);
  } else {
    lcd.print(tripInt);
  }
  lcd.print(".");
  if (tripDec < 10) lcd.print("0");
  lcd.print(tripDec);
}

// ========== FUNGSI TRANSFER DATA KE ESP ==========
void transfer() {
  time3 = millis() / 1000;
  int menit = time3 / 60;
  int detik = time3 % 60;
  
  // Hitung kecepatan mentah dan apply average filter
  int rawSpeed = (int)((RPM * kkbanspd) * 60);
  speedAverage = calculateAverageSpeed(rawSpeed);
  int kecepatan = speedAverage;  // Gunakan nilai yang sudah difilter
  
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
  
  // UPDATE LCD dengan fungsi baru
  updateLCD(kecepatan, rpm_coil, AFR_etanol, ThrottleStatus, Tc, menit, detik);
  
  // Kedip LED
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
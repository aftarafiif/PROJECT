#include <Simpletimer.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <math.h>
LiquidCrystal_I2C lcd (0x27, 16, 2);

//SENSOR KECEPATAN
const int IRSensorPin = 2;       // Pin sensor VSS (terhubung ke interrupt 0)
int inputState;
int lastInputState    = LOW;
long lastDebounceTime = 0;
long debounceDelay    = 0.5;
double kkbanspd = 0.00039878; //proto baru
//double kkbanspd = 0.0002658533; //urban magnet 6 ring 16
long endTime;
long startTime;
int RPM         = 0;
double trip     = 0;
double jarak    = 0;
int    banter   = 0;
float lnTime    = 0.5;

//SENSOR SUHU
const int ThermistorPin = A0 ;
int Vo;
float R1 = 9400;
float logR2, R2, T, Tf;
int Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

//SENSOR RPM
const int sensorRPMCoil = 3;
int inputState2;
int lastInputState2 = LOW;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 0.5;
unsigned long Time2;
unsigned long endTime2;
unsigned long startTime2;
float lnTime2 = 0;
int RPMCoil = 0;
int dontol;


//SENSOR AFR
int sensorPin = A1 ;
float afrVoltage = 0.0;   // Tegangan dari sensor AFR
float AFR_etanol = 0.0;   // Hasil AFR untuk etanol


//SENSOR TPS
const int tpsPin = A2;  // Pin analog TPS
const float voltMin = 0.45;  // Tegangan minimum TPS
const float voltMax = 4.53;  // Tegangan maksimum TPS
float tpsVoltage = 0.0;    // Tegangan dari sensor TPS
int ThrottleStatus = 0;    // Persentase bukaan throttle

//waktu
long time3;
long time;

Simpletimer timer1{};

void setup() {
  Serial.begin(9600);
  //VSS
  pinMode(IRSensorPin, INPUT);
  endTime = 0;
  //attachInterrupt(digitalPinToInterrupt(vssPin), countPulse, FALLING);
  //SUHU
  pinMode(ThermistorPin, INPUT);
  //RPM
  pinMode(sensorRPMCoil, INPUT);
  



  //lcd
  lcd.init();
  lcd.backlight();
  lcd.print("SEMERU TEAM");
  lcd.setCursor(0, 0);
  lcd.clear();
  delay(2000);

  timer1.register_callback(transfer);
}

void loop() {
  timer1.run(500);

  //
  time = millis();
  int currentSwitchState = digitalRead(IRSensorPin);

  if (currentSwitchState != lastInputState) 
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentSwitchState != inputState) {
      inputState = currentSwitchState;
      if (inputState == HIGH) {
        calculateRPM(); // Real RPM from sensor
      }
    }
  }
  lastInputState = currentSwitchState;
//RPM
int currentSwitchState2 = digitalRead(sensorRPMCoil);

  if (currentSwitchState2 != lastInputState2) 
  {
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    if (currentSwitchState2 != inputState2) 
    {
      inputState2 = currentSwitchState2;
      if (inputState2 == HIGH) {
        hitungRPMCOil(); // Real RPM from sensor
      }
     
    }
  }
  lastInputState2 = currentSwitchState2;
}


void AFR() {
  int sensorValue = analogRead(sensorPin);  // Baca nilai analog dari sensor AFR
  afrVoltage = sensorValue * (5.0 / 1023.0);  // Konversi ke tegangan (0–5V)

  // Hitung AFR untuk bensin (misal nilai linear)
  float AFR_bensin = 2.375 * afrVoltage + 7.3125;
  
  // Hitung lambda (untuk AFR Bensin)
  float lamda = AFR_bensin / 14.7;  

  // Hitung AFR untuk etanol (dalam rumus lambda * 9.12)
  AFR_etanol = lamda * 9.12;

  
  //tampilan hasil
  //Serial.print("AFR Voltage: ");
  //Serial.print(afrVoltage, 2);  // 2 digit desimal
  //Serial.print(" V | AFR Etanol: ");
  Serial.print(AFR_etanol, 2);  // AFR Etanol dengan 2 digit desimal
}

// Pin sensor TPS

void TPS() {
  int tpsValue = analogRead(tpsPin);  // Baca nilai analog dari TPS
  tpsVoltage = tpsValue * (5.5 / 930.0);  // Konversi ke voltase (0–5V)

  // Hitung persen bukaan throttle
  ThrottleStatus = (tpsVoltage - voltMin) / (voltMax - voltMin) * 100;
  ThrottleStatus = constrain(ThrottleStatus, 0, 100);  // Batasi ThrottleStatus antara 0 dan 100

  
  // Tampilkan data di Serial Monitor
  //Serial.print("TPS Value: ");
  Serial.print(ThrottleStatus);  // Menampilkan nilai ThrottleStatus
  
  /*
  // Tampilkan data di Serial Monitor
  Serial.print("TPS Voltage: ");
  Serial.print(tpsVoltage, 2);
  Serial.print(" V | Throttle: ");
  Serial.print(ThrottleStatus);
  Serial.println(" %");
  */
}




void SUHU() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.1);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc = T - 273.15;
  Tf = (Tc * 9.0)/ 5.0 + 32.0;
  Serial.print(Tc);
}

void calculateRPM() {    
  startTime = lastDebounceTime;
  lnTime = startTime - endTime;
  RPM = 60000 / (startTime - endTime);
  endTime = startTime;
  trip++;
}

void hitungRPMCOil()
{
  startTime2 = lastDebounceTime2;
  lnTime2 = startTime2 - endTime2;
  RPMCoil = 60000 / (startTime2 - endTime2);
  endTime2 = startTime2;
}

void koil(){
  int dontol = RPMCoil * 2;//RPM coil
  Serial.print(dontol);
  RPMCoil = 0;
}


void kecepatan(){
  int banter   = ((RPM * kkbanspd) *60);//kecepatan
  Serial.print(banter);
  RPM = 0; 
}

void waktu() {
  time3        = millis() / 1000;//waktu
  int menit    = time3 / 60;
  int detik    = time3 % 60;
  Serial.print(menit);
  Serial.print(":");
  Serial.print(detik);
}

void transfer() {
  time3        = millis() / 1000;//waktu
  int menit    = time3 / 60;
  int detik    = time3 % 60;
  int temperature = temperature - 273.15;//suhu
  time3 = time / 1000;
  int minutes = time3 / 60;
  int seconds = time3 % 60;

  //banter
  int banter   = ((RPM * kkbanspd) *60);
  //int rpm = (count * 60.0) / pulsesPerRevolution;

  /*
  calculateRPM();
  Rpm();
  AFR();
  TPS();
  SUHU();
  */

  
  Serial.print("#S1#");
  koil();
  //lcd.setCursor(0, 0);
 
  Serial.print("#F1##S2#");
  kecepatan();
  //Serial.println ( " C (intake)");

  Serial.print("#F2##S3#");
  AFR();
  //Serial.println ( " C (intake)");
 
  Serial.print("#F3##S4#");
  TPS();

  Serial.print("#F4##S5#");
  SUHU();

  Serial.print("#F5##S6#");
  waktu();

  Serial.println("#F6#");
  delay(0);
  
  
  
  /*
  
  Serial.print("#S1#");
  kecepatan();

  Serial.print("#F1##S2#");
  koil();

  Serial.print("#F2##S3#");
  kecepatan();

  Serial.print("#F3##S4#");
  koil();

  Serial.print("#F4##S5#");
  AFR();

  Serial.print("#F5##S6#");
  TPS();

  Serial.print("#F6##S7#");
  SUHU();

  Serial.println("#F7#"); 
  
  */
  //data

  /*
  Serial.print("#S1#");
  kecepatan();

  Serial.print("#S2#");
  Rpm();

  Serial.print("#S3#");
  AFR();

  Serial.print("#S4#");
  TPS();

  Serial.print("#S5#");
  SUHU();

  Serial.println("#F5#");
  */
  //lcd

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print(banter);

  lcd.setCursor(2,0);
  lcd.print("|");
  lcd.setCursor(2,1);
  lcd.print("|");

  lcd.setCursor(3, 1);
  lcd.print(Tc);
  lcd.setCursor(3,0);
  lcd.print(dontol);

  lcd.setCursor(7,0);
  lcd.print("|");
  lcd.setCursor(7,1);
  lcd.print("|");

  lcd.setCursor(8,0);
  lcd.print(ThrottleStatus);
  lcd.print(" %");
  lcd.setCursor(13,0);
  lcd.print(AFR_etanol, 1);  // Misal 1 digit desimal
  
  lcd.setCursor(12,0);
  lcd.print("|");
  
  lcd.setCursor(8,1);
  lcd.print(minutes);
  lcd.setCursor(10,1);
  lcd.print(":");
  lcd.setCursor(11, 1);
  lcd.print(seconds); 
/*
  lcd.setCursor(0,1);
  lcd.print("Kec:");
  lcd.setCursor(4,1);
  lcd.print(speedKmh);

  lcd.setCursor(0, 2);
  lcd.print("Trp:");
  lcd.setCursor(4, 2);
  //lcd.print(rpm);

  lcd.setCursor(0, 3);
  lcd.print("Temp:");
  lcd.setCursor(5, 3);
  lcd.print(Tc);
/*
  lcd.setCursor(11, 0);
  lcd.print(minutes);
  lcd.setCursor(13, 0);
  lcd.print(":");
  lcd.setCursor(14, 0);
  lcd.print(seconds); 
  

  lcd.setCursor(0, 0);
  lcd.print("RPM:");
  lcd.setCursor(4, 0);
  lcd.print(rpmValue);

  lcd.setCursor(11, 0); 
  lcd.print("AFR:");    
  lcd.setCursor(15, 0);
  lcd.print(AFR_etanol, 1);  // Misal 1 digit desimal

  lcd.setCursor(11, 1); 
  lcd.print("Vol:"); 
  lcd.setCursor(15,1); 
  lcd.print(tpsVoltage); 

  lcd.setCursor(11, 2);  
  lcd.print("Open:");    
  lcd.setCursor(16, 2);  
  lcd.print(ThrottleStatus);
  lcd.print(" %");
*/


  


  



  
}
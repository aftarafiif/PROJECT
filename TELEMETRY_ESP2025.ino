
#include <SoftwareSerial.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
//#include <SD.h>
//#define THINGER_SERIAL_DEBUG
#define USERNAME "davinonathan"
#define DEVICE_ID "node_mcu3"
#define DEVICE_CREDENTIAL "PrK$-!CrZm&yA3Me" 
#define SSID "Damn"
#define SSID_PASSWORD "12345678"


ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);


SoftwareSerial espSerial(3, 1);//SoftwareSerialespSerial ( rx, tx); 
String myString = "";
boolean stringComplete = false;
char c;
int Index1,Index2,Index3,Index4,Index5,Index6,Index7;
String v1, v2, v3, v4, v5, v6;

//File myFile;

void setup(){              
  Serial.begin(9600);
 /* lcd.begin();
  lcd.setCursor(0,0);
  lcd.print("SEMERU");
  delay(2000);
  lcd.clear(); */
  //espSerial.begin(9600);`
   thing.add_wifi(SSID, SSID_PASSWORD);

   thing["COBA"] >> [](pson& out){
    // Add the values and the corresponding code
    out["koil"]             = v1;
    out["kecepatan"]       = v2;
    out["AFR"]             = v3;
    out["TPS"]             = v4;
    out["SUHU"]            = v5;
    out["waktu"]           = v6;
   }; 
   Serial.println(WiFi.localIP());
}
void loop(){
  thing.handle();
 //thing.write_bucket("TES", "COBA");
//  sleep the device SLEEP_MS milliseconds
 // ESP.deepSleep(1000, WAKE_RF_DEFAULT);
  
while(Serial.available()>0)
{

  //Serial.println("test");
  //delay(10);
  c = Serial.read();
  
  myString += c;
 //Serial.println(myString);
  
  int lentstring = myString.length();
  String batas = myString.substring(lentstring-4, lentstring);
  int lenstring1 = batas.length();
  //Serial.println("batas>>>"+batas);
if (batas == "#F6#")
{
  Serial.println("batas = true");
 stringComplete = true;
  }
if (stringComplete >0)
{
  Serial.println("--Stringcomplete---");
Index1 = myString.indexOf("#S1#");
Index2 = myString.indexOf("#F1##S2#", Index1+1);
Index3 = myString.indexOf("#F2##S3#", Index2+1);
Index4 = myString.indexOf("#F3##S4#", Index3+1);
Index5 = myString.indexOf("#F4##S5#", Index4+1);
Index6 = myString.indexOf("#F5##S6#", Index5+1);
Index7 = myString.indexOf("#F6#", Index6+1);
delay(5);
v1 = myString.substring(Index1+4, Index2);
v2 = myString.substring(Index2+8, Index3);
v3 = myString.substring(Index3+8, Index4);
v4 = myString.substring(Index4+8, Index5);
v5 = myString.substring(Index5+8, Index6);
v6 = myString.substring(Index6+8, Index7);

//Serial.println(Index1);
Serial.print("Rpm          :");Serial.println(v1);
Serial.print("kecepatan    :");Serial.println(v2);
Serial.print("AFR          :");Serial.println(v3);
Serial.print("TPS          :");Serial.println(v4);
Serial.print("SUHU         :");Serial.println(v5);
Serial.print("waktu         :");Serial.println(v6);
Serial.println("========================");
  
/*
lcd.clear();
lcd.setCursor(0, 0);lcd.print("kec  =");
lcd.setCursor(6,0);lcd.print(v1);
lcd.setCursor(0,1);lcd.print("jarak=");
lcd.setCursor(6,1);lcd.print(v2);
lcd.setCursor(11,0);lcd.print(v3);
*/

myString="";
}
if (stringComplete) 
{
//Serial.println(myString);
myString = "";
stringComplete = false;
}
}
}

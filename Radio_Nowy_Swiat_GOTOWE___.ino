// Audio output : GND and RX
//  I2C connection LCD 16x2: D2 - SDA D1 - SLC
// Processor speed 160Mhz
//
#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
    #include <HTTPClient.h>
#else
    #include <ESP8266WiFi.h>
      #include <DNSServer.h>
#include <ESP8266WebServer.h>
#endif
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include "AudioFileSourceICYStream.h"   // https://github.com/earlephilhower/ESP8266Audio
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//#include <EEPROM.h>
#include <Arduino.h>
WiFiServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
char URL[100]="http://n04a-eu.rcs.revma.com/ypqt40u0x1zuv";
char url[100];
AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;
void setup(){
 //      EEPROM.begin(512);
  Serial.begin(115200);
    // initialize the LCD
  lcd.begin();
 lcd.setCursor(0,0);
 lcd.print("Radio Nowy Swiat");
  delay(1000);
  Serial.println("Connecting to WiFi");
lcd.clear();
lcd.print("Connecting WiFi");
 delay(1000);
   lcd.clear();
  lcd.print("AutoConnectAP");
  lcd.setCursor(0,1);
  lcd.print ("//192.168.4.1/");
  delay (500);
 WiFiManager wifiManager;
   // Uncomment and run it once, if you want to erase all the stored information
//wifiManager.resetSettings();
  wifiManager.autoConnect("AutoConnectAP");
  lcd.setCursor(0,1);
  lcd.print("Connected");
  Serial.print("Connected");
  lcd.clear();
lcd.print("Radio Nowy Swiat");
delay(1000);
 //------------------REDIRECT PROCEDURE-------------------------
 const char* headerNames[] = { "Location" };
    String Location;
    int pos1, pos2;
  HTTPClient http;   //HTTP OPENING
    http.begin(URL); //HTTP Connecting
    http.collectHeaders(headerNames, sizeof(headerNames)/sizeof(headerNames[0]));
 Serial.println("headerNames  ");   Serial.print(headerNames[0]);
 Serial.print("headerNames 0  ");Serial.print(headerNames[0]);
// Serial.print sizeof(headerNames[0]);
    int httpCode = http.GET();    // Get error code
Serial.print(httpCode);
    //------- follow redirect -----------
    if (httpCode == 302) // Its mean - stream is redirect
    {
 lcd.setCursor (0,0);     
lcd.print("   Following"); 
 lcd.setCursor (0,1); 
 lcd.print("   redirect"); 
Serial.println("Location"); 
      Location = http.header("Location");// New stream adress
Serial.println(Location); 
    }
    else
    {
Location = URL;
    }
    http.end();

 //----------------STRING TO CHAR------------------------

 Location.substring(0, Location.length()+1).toCharArray(url, Location.length()+1);
 
//-------------------Connectin to stream---------------------------

  file = new AudioFileSourceICYStream(url);
  file->RegisterMetadataCB(MDCallback, (void*)"ICY");
  buff = new AudioFileSourceBuffer(file, 4096);
  buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  mp3->begin(buff, out);
  lcd.clear(); // Clear any comments from LCD
}
void loop()
{
  static int lastms = 0;

  if (mp3->isRunning()) {
    if (millis()-lastms > 400) { // Every 350 ms Scroling display.
      lastms = millis();
     lcd.scrollDisplayLeft();
//      Serial.printf("Running for %d ms...\n", lastms);
      Serial.flush();
     }
    if (!mp3->loop()) mp3->stop();
  } else {
    
   Serial.printf("MP3 done\n");
   lcd.print ("RECONNECTION");
    delay(1000);
ESP.restart();
  }
}
// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
// Note that the type and string may be in PROGMEM, so copy them to RAM for printf
//  char s1[32], s2[64];
//  strncpy_P(s1, type, sizeof(s1));
//  s1[sizeof(s1)-1]=0;
//  strncpy_P(s2, string, sizeof(s2));
//  s2[sizeof(s2)-1]=0;
//  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
String mystring(string);
mystring.replace("ę", "ę");mystring.replace("ó", "o");
mystring.replace("ą", "ą");mystring.replace("Ś", "S");
mystring.replace("ś", "s");mystring.replace("Ł", "L");
mystring.replace("ł", "ł");mystring.replace("Ż", "Z");
mystring.replace("Ź", "Z");mystring.replace("Ć", "C");
mystring.replace("ń", "n");
lcd.setCursor(0,1);lcd.print(mystring);
lcd.setCursor(0,0);lcd.print("Radio Nowy Swiat    Radio Nowy Swiat  ");
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}

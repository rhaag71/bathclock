#include <Arduino.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
/**
 * GLOBAL PIN CONFIGURATION
 */
const int TFT_CS = 15;
const int TFT_DC = 4;
const int TFT_MOSI = 23;
const int TFT_SLK = 18;
const int TFT_RST = 2;
const int TFT_LED = 19;     
const int BUZZER = 5;
const int NIGHTLIGHT = 17;
const int LUX_SDA = 21;
const int LUX_SCL = 22;
const int DHT_OUT = 27;

/**
 * EEPROM libraries and resources
 */
#include "EEPROM.h"
#define EEPROM_SIZE 64
 
/**
 * ILI9341 TFT libraries and resources
 */
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans24pt7b.h"
//                                      cs  dc  mosi slk rst
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SLK, TFT_RST);

#define ILI9341_LCYAN   0x0418
#define ILI9341_LORANGE 0xFC08
#define ILI9341_LGREEN  0x87F0
#define ILI9341_LGRAY   0x8410
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198

/**
 * BH1750 Lux meter libraries and resources
 */
#include <BH1750FVI.h>
// Create the Lightsensor instance
BH1750FVI LightSensor(BH1750FVI::k_DevModeContHighRes); 

/**
 * DHT-11 Temp and humidity sensor  libraries and resources
 */
#include "DHT.h"
#define DHTTYPE DHT11
DHT dht(DHT_OUT, DHTTYPE);       

/**    
 *  WIFI Libraries and resources   
 */
#include <WiFi.h>
//Absolute path to file containing WiFi credentials
const char* host = "bathclock";
const char* ssid       = "Gamma-Ray";
const char* password   = "radiation543";
const int connTimeout = 10; //Seconds

WebServer server(80);

/*
 * Login page
 */
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

/** 
 *  TIME libraries and resources
 */
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

/**
 * For rounding values
 */
#include "math.h"

/**
 * PWM Constants
 */
const int freq = 5000;
const int tftledChannel = 0;
const int resolution = 8;

/**
 * TFT Display Constants 
 */
const int xTime = 10;
const int yTime = 75;
const int tftBG = ILI9341_BLACK;
const int tftTimeFG = ILI9341_RED;
 
/**
 * GLOBALS
 */
String   prevTime = "";
String   currTime = "";
String   prevDate = "";
String   currDate = ""; 
uint16_t prevLux = 0;
uint16_t currLux = 0;
float    prevTemp = 0;
float    currTemp = 0;
float    prevHumi = 0;
float    currHumi = 0;
bool     onWifi = false;
String   weekDays[] = {"", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"};
String   months[] = {"", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
const unsigned long bathroomTimeInterval = 240000; // 4 minutes
unsigned long previousMillis;
unsigned long currentMillis;

//Configurable values
int      bootWait; //Seconds
byte     maxLux;
byte     minLux;
byte     minBrightness;

// forward declarations
void loadConfiguration();
void setBGLuminosity(int level);
void nightLight(int mode);
bool wifiConnect();
bool getNtpTime();
void displayTime();
void displayDate();
void displayTemp();
void displayHumi();
// void displayLux();
uint16_t getCurrentLux();
float getCurrentTemp();
float getCurrentHumi();
int returnCurrHour();
void refreshTime();
String hourMinuteToTime(int hour, int minute);
void timeChanged(String prevTime, String currTime);
void dateChanged(String prevDate, String currDate);
void luxChanged(uint16_t prevLux, uint16_t currLux);
void tempChanged(float prevTemp, float currTemp);
void humiChanged(float prevHumi, float currHumi);

byte getBootWait();
void setBootWait(byte value);
byte getMaxLux();
byte getMinLux();
byte getMinBrightness();

void setMaxLux(byte value);
void setMinLux(byte value);
void setMinBrightness(byte value);

void calculateAndSetBGLuminosity(uint16_t currLux);


void setup() {
  /**
   * Serial port
   */
  Serial.begin(115200);
  
  /**
   * EEPROM
   */
  EEPROM.begin(EEPROM_SIZE);

  /**
   * Loads EEPROM configuration
   */
  loadConfiguration();
  
  /**
   * TFT DISPLAY
   */
  //Background light PWM
  ledcSetup(tftledChannel, freq, resolution);
  ledcAttachPin(TFT_LED, tftledChannel);
  //Start with high intensity
  setBGLuminosity(255);
  tft.begin();
  tft.setRotation(3);
  yield();
  
  //Boot screen
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("You no sleep on toilet!");
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.println("");
  tft.println("Booting...");
  tft.println("Setting up devices...");


  /**
   * BUZZER
   */
  pinMode(BUZZER, OUTPUT);

  /**
   * NIGHTLIGHT
   */
  pinMode(NIGHTLIGHT, OUTPUT);  

  /**
   * Light sensor
   */
  LightSensor.begin(); 

  /**
   * Temperature and humidity sensor
   */
  dht.begin();

  /**
   * Wifi connect
   */
  tft.print("Connecting to WiFi AP "); 
  tft.println(ssid);
  
  wifiConnect();
  if(onWifi == true){
    tft.print("   Connection succeed, obtained IP ");
    tft.println(WiFi.localIP());
  }else{
    tft.println("*** Connection failed. Unexpected operation results.");
  }
  tft.println("Obtaining NTP time from remote server...");

  /**
   * NTP Time
   */
  getNtpTime();
  delay(100); //We need a delay to allow info propagation
  

  
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  Serial.println("\n   ");


  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  tft.println("Ready...");
  /**
   * Blink night light for "ready..."
   */
  digitalWrite(NIGHTLIGHT, HIGH);
  delay(350);
  digitalWrite(NIGHTLIGHT, LOW);
  delay(350);
  digitalWrite(NIGHTLIGHT, HIGH);
  delay(350);
  digitalWrite(NIGHTLIGHT, LOW);

  //Prepare screen for normal operation
  setBGLuminosity(0);
  tft.fillScreen(ILI9341_BLACK);
  yield();

  //Paint elements
  displayTime();
  displayDate();
  displayTemp();
  displayHumi();
  // displayLux(); // for debug if needed
  setBGLuminosity(255);
  /** 
   *  TESTING
   */
}

void loop() {
  getCurrentLux();
  getCurrentTemp();
  getCurrentHumi();
  refreshTime();
  server.handleClient();
  delay(100);
}

/**
 * Connects to WIFI
 */
bool wifiConnect(){
  onWifi = false;
  int retries = 0;
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      retries++;
      if(retries == (connTimeout * 2)){
        Serial.println(" TIMEOUT");
        break;
      }
  }
  if(WiFi.status() == WL_CONNECTED){
    onWifi = true;
    Serial.println(" CONNECTED");
  }
  return onWifi;
}

/**
 * Obtains time from NTP server
 */
bool getNtpTime(){
  bool result = false;
  if(onWifi == true){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    result = true;
  }else{
    Serial.println("getNtpTime: Not connected to wifi!"); 
  }
  return result;
}

/**
 * Returns a string formatted HH:MM based on hours and minutes
 */
String hourMinuteToTime(int hour, int minute){
  String sTime;
  char cTime[12]=" ";
  if(hour==0){
    hour=12;
    sprintf(cTime, "%02d:%02d AM", hour, minute);
  }
  else if(hour==12){
      hour=12; 
      sprintf(cTime, "%02d:%02d PM", hour, minute);
  }
  else if(hour<12&&hour!=0){
    if(hour>9) {
      sprintf(cTime, "%02d:%02d AM", hour, minute);
    } else {
      sprintf(cTime, " %01d:%02d AM", hour, minute);
    }
  }
  else if(hour>12&&hour!=0)
  { 
      hour=hour-12;
      if(hour>9) {
        sprintf(cTime, "%02d:%02d PM", hour, minute);
      } else {
        sprintf(cTime, " %01d:%02d PM", hour, minute);
      }
  }
  sTime = (char*)cTime; 
  return sTime;
}

/*
 * Returns current time in HH:MM format
 */
void refreshTime(){
  //Time
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now); 
  prevTime = currTime;
  currTime = hourMinuteToTime(timeinfo->tm_hour, timeinfo->tm_min); 
  if(prevTime != currTime){
    timeChanged(prevTime, currTime);
    //If time has changed, lets check if date has changed too
    //Date
    int wDay;
    wDay = timeinfo->tm_wday;
    if(wDay == 0){
      wDay = 7;
    }
    String calDate = "";
    calDate = calDate + weekDays[wDay];
    calDate = calDate + "  ";
    calDate = calDate + months[(timeinfo->tm_mon + 1)];
    calDate = calDate + " ";
    calDate = calDate + timeinfo->tm_mday;
    calDate = calDate + ", ";  
    calDate = calDate + (timeinfo->tm_year + 1900);
    if(calDate.length() == 21){
      calDate = " " + calDate;
    }
    prevDate = currDate;
    currDate = calDate;
    if(prevDate != currDate){
      dateChanged(prevDate, currDate);
    }
  }
}

/**
 * Return the current hour
 */
int returnCurrHour() {
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  // Serial.println(timeinfo->tm_hour); // debug
  return timeinfo->tm_hour;
}

/**
 * Displays time string erasing the previous one
 */
void displayTime(){
  tft.setFont(&FreeSans18pt7b);
  tft.setTextSize(2);
  tft.setCursor(xTime, yTime);
  tft.setTextColor(tftBG);
  yield();
  tft.println(prevTime);
  yield();
  tft.setCursor(xTime, yTime);
  tft.setTextColor(tftTimeFG);
  yield();
  tft.println(currTime);
  yield();
  tft.setFont();
  yield();
}
/**
 * Displays date string 
 */
void displayDate(){
  tft.fillRect(14, 94, 192, 30, ILI9341_BLACK);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1.5);
  tft.setCursor(40, 115);
  tft.setTextColor(ILI9341_LIGHTGREY);
  yield();
  tft.println(currDate);
  yield();
  tft.setFont();
  yield();
}

/**
 * Displays temperature
 */
void displayTemp(){
  int bgColor = 0;
  int rndTemp = 0;
  bgColor = ILI9341_BLACK;
  tft.setTextColor(ILI9341_WHITE);
  yield();
  tft.fillRect(25, 124, 100, 100, bgColor);
  tft.drawRect(25, 124, 100, 100, ILI9341_BLACK);
  yield();
  tft.setFont(&FreeSans18pt7b);
  tft.setTextSize(1.5);
  tft.setCursor(40, 175);
  rndTemp = round(currTemp);
  tft.print(rndTemp);
  tft.print("  F");
  tft.setFont();
  tft.setCursor(82, 145);
  tft.setTextSize(2);
  tft.print(char(247));
  tft.setTextColor(ILI9341_BLACK);
}

/**
 * Displays relative humidity
 */
void displayHumi(){
  int bgColor = 0;
  int rndHumi = 0;
  bgColor = ILI9341_BLACK;
  tft.setTextColor(ILI9341_WHITE);
  yield();
  tft.fillRect(130, 124, 140, 100, bgColor);
  tft.drawRect(130, 124, 140, 100, ILI9341_BLACK);
  yield();
  tft.setFont(&FreeSans18pt7b);
  tft.setTextSize(1.5);
  tft.setCursor(160, 175);
  rndHumi = round(currHumi);
  tft.print(rndHumi);
  tft.print("%");
  tft.setFont();
}

/**
 * Displays lux (debug if needed)
 * if used uncomment forward declaration at top of code
 */
// void displayLux(){
  // int bgColor = 0;
  // bgColor = ILI9341_BLACK;
  // tft.setTextColor(ILI9341_WHITE);
  // yield();
  // tft.fillRect(212, 170, 92, 60, bgColor);
  // tft.drawRect(212, 170, 92, 60, ILI9341_BLACK);
  // yield();
  // tft.setFont(&FreeSans9pt7b);
  // tft.setTextSize(1.5);
  // tft.setCursor(214, 200);
  // char cLux[5]=" ";
  // sprintf(cLux, "%05d", currLux);
  // tft.print(cLux);
  // tft.setFont();
// }

/**
 * Sets TFT background luminosity (0-255)
 */
void setBGLuminosity(int level){
  ledcWrite(tftledChannel, level);
}

/**
 * Play alarm noise
 */
void playAlarm() {
  Serial.println("playing alarm..");
  for(int i=0; i< 200; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(1);
    digitalWrite(BUZZER, LOW);
    delay(1);
  }
}

/**
 * Turn on nightlight if lights out, off if lights on
 */
void nightLight(int mode) {
  // Ambient lights are off
  if(mode==1) {
    digitalWrite(NIGHTLIGHT, HIGH);
    // delete countdown indicator circle
    tft.fillCircle(300, 220, 3,ILI9341_BLACK);
    tft.drawCircle(300, 220, 3,ILI9341_BLACK);
    previousMillis = millis();
  } 
  
  // Ambient lights are on
  if(mode==0) {
    digitalWrite(NIGHTLIGHT, LOW);
    int thisHour = returnCurrHour();
    currentMillis = millis();

    if (thisHour<=6) {
        // display countdown indicator circle, showing that there is an alarm countdown happening
        tft.fillCircle(300, 220, 3,ILI9341_YELLOW);
        tft.drawCircle(300, 220, 3,ILI9341_WHITE);
      if (currentMillis - previousMillis >= bathroomTimeInterval) {
        playAlarm();
      }
    }
  }
}

/**
 * Returns ambient light luxes 
 */
uint16_t getCurrentLux(){
  prevLux = currLux;
  currLux = LightSensor.GetLightIntensity();
  if(prevLux != currLux){
    Serial.println(currLux);
    luxChanged(prevLux , currLux);
  }
  return currLux;
}

/**
 * Calculates and sets screen backlight brightness
 */
void calculateAndSetBGLuminosity(uint16_t currLux){
  int finalLux = currLux;
  if(finalLux > maxLux){
    finalLux = maxLux;
  }
  if(finalLux < minLux){
    finalLux = minLux;
  }
  double levelsWidth = maxLux - minLux;
  double level = finalLux - minLux;
  double ratio = level / levelsWidth;
  double brightnessWidth = 255 - minBrightness;
  int brightnessValue = (brightnessWidth * ratio) + minBrightness;
  setBGLuminosity(brightnessValue);
}

/**
 * Returns temperature
 */
float getCurrentTemp(){
  prevTemp = currTemp;
  currTemp =  (dht.readTemperature()*1.8)+32;
  if(prevTemp != currTemp){
    tempChanged(prevTemp , currTemp);
  }
  return currTemp;
}

/**
 * Returns humidity
 */
float getCurrentHumi(){
  prevHumi = currHumi;
  currHumi = dht.readHumidity();
  if(prevHumi != currHumi){
    humiChanged(prevHumi , currHumi);
  }
  return currHumi;
}

/**
 *  EVENTS
 */

/**
 * Event for change of time HH:MM
 */
void timeChanged(String prevTime, String currTime){
  //Serial.println("timeChanged event fired!");
  displayTime();
}

/**
 * Event for change of date weekDay, day de Month de Year
 */
void dateChanged(String prevDate, String currDate){
  //Serial.print("dateChanged event fired! ");
  //Serial.println(currDate);
  displayDate();
}

/**
 * Event for change of ambient light
 */
void luxChanged(uint16_t prevLux, uint16_t currLux){
  //Serial.print("luxChanged event fired! ");
  //Serial.println(currLux);
  if(currLux<80) {
    int mode = 1;
    nightLight(mode);
  }
  if(currLux>180) {
    int mode = 0;
    nightLight(mode);
  }
  calculateAndSetBGLuminosity(currLux);
}

/**
 * Event for change of temperature
 */
void tempChanged(float prevTemp, float currTemp){
  //Serial.print("tempChanged event fired! ");
  //Serial.println(currTemp);
  displayTemp();
}

/**
 * Event for change of humidity
 */
void humiChanged(float prevHumi, float currHumi){
  //Serial.println("humiChanged event fired!");
  // Serial.println(currHumi);
  displayHumi();
}

/**
 * Load Configuration from EEPROM
 */
void loadConfiguration(){
  bootWait = getBootWait();
  maxLux = getMaxLux();
  minLux = getMinLux();
  minBrightness = getMinBrightness(); 
}

/**
 * Boot Wait
 * Address 0
 */
byte getBootWait(){
  byte value = byte(EEPROM.read(0));
  if(value == 255){
    value = 20;
  }
  return value;
}
void setBootWait(byte value){
  EEPROM.write(0, value);
  EEPROM.commit();
  bootWait = value;
}

/* maxLux
 * Address 8
 */
byte getMaxLux(){
  byte value = byte(EEPROM.read(8));
  if(value == 255){
    value = 20;
  }
  return value;
}
void setMaxLux(byte value){
  EEPROM.write(8, value);
  EEPROM.commit();
  maxLux = value;
  calculateAndSetBGLuminosity(currLux);
}

/* minLux
 * Address 9
 */
byte getMinLux(){
  byte value = byte(EEPROM.read(9));
  if(value == 255){
    value = 0;
  }
  return value;
}
void setMinLux(byte value){
  EEPROM.write(9, value);
  EEPROM.commit();
  minLux = value;
  calculateAndSetBGLuminosity(currLux);
}

/* minBrightness
 * Address 10
 */
byte getMinBrightness(){
  byte value = byte(EEPROM.read(10));
  if(value == 255){
    value = 10;
  }
  return value;
}
void setMinBrightness(byte value){
  EEPROM.write(10, value);
  EEPROM.commit();
  minBrightness = value;
  calculateAndSetBGLuminosity(currLux);
}

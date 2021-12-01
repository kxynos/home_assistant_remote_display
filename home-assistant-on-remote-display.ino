/**************************************************************************

  Home Assistant Radon sensor shown on ESP8266 and SSD1306 display

  Coded by Konstantinos Xynos (2021, MIT License)
 
 **************************************************************************/

// WIFI end
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;
// WIFI end

#include <ArduinoJson.h>

// Display 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Display end

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define NUMFLAKES_EXIT     5 // Number of snowflakes in the animation example

#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

// http://dot2pic.com/
  static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000,0b00000000,
0b00000111,0b11100000,
0b00011000,0b00011000,
0b00100000,0b00000100,
0b01000100,0b00100010,
0b10001110,0b01110010,
0b10000100,0b00100001,
0b10000001,0b10000001,
0b10000000,0b00000001,
0b01110000,0b00001110,
0b00001010,0b01010000,
0b01101111,0b11110110,
0b10011111,0b11111101,
0b01000000,0b00000010,
0b10011111,0b11111101,
0b01100000,0b00000110};
  

#define AUTHORZ "Bearer HOME_ASSISTANT_LONG_TOKEN" // set the Home Assistant Long Token here 

String radon_value_changed ;
String radon_value_changed_date;
String radon_value_changed_time;
int radon_value_int;
int radon_value_int_previous = 0;
int radon_value_int_current = 0;

//change this as needed 
int RADON_WARNING_LIMIT = 145; // can update this
//int RADON_WARNING_LIMIT = 80;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  //display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  //display.display();
  //delay(2000);
  testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps

  delay(100);
  bootmsg();
  //delay(2000);
  Serial.println("[!] I2C display device setup and running !");

//wifi section

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    display.print(F("[SETUP] WAIT "));
    display.print(t);
    display.println(F("... "));
    display.display();
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "WIFI_PSK"); // set the WIFI SSID and PASSPHRASE here

  radon_value_int = 0;

}

void loop(){

  wifiConnect_Req_json();
  delay(10000);

  }

void wifiConnect_Req_json(){
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    
    display.setTextSize(1);
    display.println(F("[+] Connecting to HA"));
    display.display();
    delay(1000);
    
  if (http.begin(client, "https://XXXXXXXXXXX/api/states/sensor.XXXXXX")) {  // HTTP, set the IP or host name of the Home Assistant and the sensor name
  
      http.addHeader("Authorization", AUTHORZ);
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);

          const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 450;
          DynamicJsonDocument payload_dict(capacity);
          DeserializationError error = deserializeJson(payload_dict, payload);
        
          // Test if parsing succeeds.
          if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
          }else{
            Serial.println( payload_dict["state"].as<const char*>());
            radon_value_changed = payload_dict["last_changed"].as<String>();
            radon_value_changed_date = radon_value_changed.substring(0,10);
            radon_value_changed_time = radon_value_changed.substring(11,19);
            radon_value_int = payload_dict["state"].as<int>();
            drawRadonReading();
            }
         
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  
  } 


void drawRadonReading() {
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Radon: "));
  display.println(radon_value_int);
  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.println();
  display.print("               ");
  display.println(radon_value_int_previous);
  display.print("Last chg: ");
  display.println(radon_value_changed_date);
  display.print("           ");
  display.println(radon_value_changed_time);
  
  if (radon_value_int > RADON_WARNING_LIMIT){
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.println();
    display.print(F("!! OVER "));
    display.print(RADON_WARNING_LIMIT);
    display.println(F(" LIMIT !!"));
    }

  if(radon_value_int_current != radon_value_int){
    radon_value_int_previous = radon_value_int_current;
    radon_value_int_current = radon_value_int;
  // Serial.printf("[!] Current : %d, Previous: %d\n", radon_value_int_current, radon_value_int_previous);
    }

  if(radon_value_int_previous == 0 ){
    radon_value_int_previous = radon_value_int_current;
    radon_value_int_current = radon_value_int;
    }
  // Serial.printf("[!] Current : %d, Previous: %d\n", radon_value_int_current, radon_value_int_previous);

  display.display();
  delay(2000);
}


void bootmsg() {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("[+] Booting..."));
  display.println(F("[+] Setting up wifi.."));
  
  display.display();
  delay(1000);
}

// code from adafuit
void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }
  int count = 0;
  for(; ;) { // Loop ...
    display.clearDisplay(); // Clear the display buffer

    // Draw each snowflake:
    for(f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    display.display(); // Show the display buffer on the screen
    delay(200);        // Pause for 1/10 second

    // Then update coordinates of each flake...
    for(f=0; f< NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS]   = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
        
        count = count +1;

      }
    }
    
    if(count > 4){
    break;
    }

  }
  
}
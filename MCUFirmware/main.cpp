
// includes for the OLED display
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// include for the RTC
#include "RTClib.h"

// include s for the neopixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Misc defines
#define NEOPIXEL_NUM_LEDS 3
#define NEOPIXEL_BRIGHTNESS 50
#define OLED_RESET 4
#define NEOPIXEL_BLINK_INTERVAL_MS 400
#define RTC_BATTERY_READ_INTERVAL_MS 1000  

// Pin defines
#define RTC_BAT_PIN A2
#define SOIL_HUMIDITY_SENSOR_PIN A3
#define NEOPIXEL_PIN 9

// global variables
uint32_t currentMillis = 0;

bool stripState = LOW;
uint32_t NeoPixel_previousMillis = 0;

uint32_t RTC_previousMillis = 0;
const float RTC_batResolution = 4.2 / 1024.0;
int RTC_batADCValue = 0;
float RTC_batVolt = 0;

uint16_t soilHumidity = 0;

// NeoPixel instance
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// OLED instance
Adafruit_SSD1306 display(OLED_RESET);
// RTC
RTC_DS1307 RTC;


void setup () {
  // set up the analog input for the RTC battery                 
  pinMode(RTC_BAT_PIN, INPUT);
  
  // set up the analog input for the soil humidity sensor                  
  pinMode(SOIL_HUMIDITY_SENSOR_PIN, INPUT);
    
  // NEOPIXEL init
  strip.setBrightness(NEOPIXEL_BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  
  // init RTC
  Wire.begin();
  RTC.begin();
  if (!RTC.isrunning()) {
    // sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  
    display.setTextSize(1);
    display.setTextColor(WHITE);  
    const uint32_t color = strip.Color(0, 255, 0);
    for(int i = 3; i>0; i--) {
      display.clearDisplay(); 
      display.setCursor(0,0); 
      display.println("Setting RTC to:");
      display.print(__DATE__);
      display.print("//");
      display.println(__TIME__);
      display.display();
      display.setCursor(0,40);
      display.print(i, DEC);
      display.println(" s");
      display.display();
  
      strip.setPixelColor(i-1, color);
      strip.show();
      
      delay(1000);
    }
    display.clearDisplay(); 
    display.setCursor(0,0); 
    display.println("Setting RTC to:");
    display.print(__DATE__);
    display.print("//");
    display.println(__TIME__);
    display.display();
    display.setCursor(0,40);
    display.print(0, DEC);
    display.println(" s");
    display.display();
  }
  
  // clear all pixels
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i,0,0,0);
    strip.show();
  }
  strip.show();
  delay(200);

  // Clear the buffer.
  display.clearDisplay();
}


// Set all NeoPixels to a color
void setStripColor(uint32_t color) {
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

String RTC_getDateTime(void) {
  DateTime now = RTC.now();

   /*display.print(now.day(), DEC);
    display.print('/');
    display.print(now.month(), DEC);
    display.print('/');
    display.print(now.year(), DEC);
    display.print(' ');
    display.print(now.hour(), DEC);
    display.print(':');
    display.print(now.minute(), DEC);
    display.print(':');
    display.println(now.second(), DEC); */
  
  String retVal = "";
  if(now.day() < 10) {
    retVal += '0';
    retVal += now.day(); 
  } else {
    retVal += now.day();     
  }
  retVal += '.'; 
  if(now.month() < 10) {
    retVal += '0';
    retVal += now.month(); 
  } else {
    retVal += now.month();     
  }
  retVal += '.';
  retVal += now.year();
  
  retVal += " | ";
  if(now.hour() < 10) {
    retVal += ' ';
    retVal += now.hour(); 
  } else {
    retVal += now.hour();     
  }
  retVal += ':';
  if(now.minute() < 10) {
    retVal += '0';
    retVal += now.minute(); 
  } else {
    retVal += now.minute();     
  }
  retVal += ':';
  if(now.second() < 10) {
    retVal += '0';
    retVal += now.second(); 
  } else {
    retVal += now.second();     
  }  
  return retVal;
}


void loop() {
  display.clearDisplay();    
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.println(RTC_getDateTime()); 
   
  currentMillis = millis();
  
  if (currentMillis - RTC_previousMillis >= RTC_BATTERY_READ_INTERVAL_MS) {
    RTC_previousMillis = currentMillis;
    RTC_batADCValue = analogRead(RTC_BAT_PIN);
    RTC_batVolt = round(RTC_batResolution * RTC_batADCValue * 100)/100.0;
  }
  display.print("RTC Battery: ");
  display.print(RTC_batVolt, 2);
  display.println(" V");
  display.println();
  
  display.println("Soil Humidity: ");
  
  soilHumidity = analogRead(SOIL_HUMIDITY_SENSOR_PIN);
  if(soilHumidity >= 1000) {
  display.println("NOT IN SOIL");
  
  unsigned long currentMillis = millis();
    if (currentMillis - NeoPixel_previousMillis >= NEOPIXEL_BLINK_INTERVAL_MS) {
      NeoPixel_previousMillis = currentMillis;
  
      // if the LED is off turn it on and vice-versa:
      if (stripState == LOW) {
        stripState = HIGH;
        setStripColor(strip.Color(255, 0, 0));
      } else {
        stripState = LOW;
        setStripColor(strip.Color(0, 0, 0));
      }
    }
  }
   else if(soilHumidity < 1000 && soilHumidity >= 600) { 
   display.println("DRY");
   setStripColor(strip.Color(255, 0, 0));
  }
  if(soilHumidity < 600 && soilHumidity >= 370) {
   display.println("HUMID");    
   setStripColor(strip.Color(255, 255, 0));
  }
  else if(soilHumidity < 370) {
   display.println("WET");   
   setStripColor(strip.Color(0, 255, 0));
  }
  
  display.display();
}
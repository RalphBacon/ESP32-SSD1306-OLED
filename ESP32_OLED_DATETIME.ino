#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// My secret stuff (eg WiFi password)
// #include "config.h"

// How we connect to your local wifi
#include <WiFi.h>

// See Arduino Playground for details of this useful time synchronisation library
#include <time.h>

const char* ssid = "YOUR WIFI SSID";
#define password "YOUR WIFI PASSWORD"

const char* ntpServer = "uk.pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

// WiFi specific defines
#define WifiTimeOutSeconds 10

// Days of week. Day 1 = Sunday
String DoW[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
          "Friday",
          "Saturday" };

// Months
String Months[] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
          "Oct",
          "Nov", "Dec" };

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 5, 4);

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  // Initialize OLED with the I2C addr 0x3C (for the 64x48)
  // For full details see here: https://github.com/ThingPulse/esp8266-oled-ssd1306
  display.init();
  display.clear();
  display.display();

  // Uncomment this next line if the processor is on the LEFT!
  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 10, "Initialising!");
  display.display();

  // Connect to your local wifi (one time operation)
  connectToWifi();

  //init and get the time
  struct tm timeinfo;
  bool timeOK = false;
  while (!timeOK) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      delay(1000);
    }
    else {
      timeOK = true;
    }
  }

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
  // a unsigned byte value between 0 and 100
  for (int cnt = 0; cnt < 100; cnt++) {
    display.drawProgressBar(0, 40, 120, 10, cnt);
    display.display();
    delay(30);
  }

  // Give user a chance to read display
  delay(2000);

}

void loop() {
  // Only do this every <period> seconds
  static unsigned long prevMillis = millis();

  // This just prints the "system time" every N seconds
  if (prevMillis + 999 < millis()) {
    //digitalClockDisplay();
    printLocalTime();
    prevMillis = millis();
  }
}

// -----------------------------------------------------------------------
// Establish a WiFi connection with your router
//
// Note: if connection is established, and then lost for some reason, ESP
// will automatically reconnect to last used access point once it is again
// back on-line. This will be done automatically by Wi-Fi library, without
// any user intervention.
// -----------------------------------------------------------------------
void connectToWifi() {
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Try to connect 4 times a second for X seconds before timing out
  int timeout = WifiTimeOutSeconds * 4;
  while (WiFi.status() != WL_CONNECTED && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }

  // Successful connection?
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect, exiting");
    // Set some LED failure here, for example
    return;
  }

  Serial.print("\nWiFi connected with (local) IP address of: ");
  Serial.println(WiFi.localIP());
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  // Update OLED
  displayDateTime(timeinfo);
}

void displayDateTime(struct tm timeinfo) {

  int yOffset = 0;
  int xOffset = 0;
  char value[2];
  char minString[5];

  display.clear();
  display.setFont(ArialMT_Plain_10);
  int weekday = timeinfo.tm_wday;
  display.drawString(xOffset, yOffset, DoW[weekday]);
  xOffset += display.getStringWidth(DoW[weekday]) + 5;

  display.drawString(xOffset, yOffset, Months[timeinfo.tm_mon]);
  xOffset += display.getStringWidth(Months[timeinfo.tm_mon]) + 5;
  display.drawString(xOffset, yOffset, itoa(timeinfo.tm_mday, value, 10));

  display.setFont(ArialMT_Plain_16);
  int hh = timeinfo.tm_hour;
  sprintf(minString, "%02d", hh);
  yOffset = 20;
  display.drawString(1, yOffset, minString);
  display.drawString(20, yOffset, ":");

  int mm = timeinfo.tm_min;
  sprintf(minString, "%02d", mm);
  display.drawString(23, yOffset, minString);
  display.drawString(41, yOffset, ":");

  int ss = timeinfo.tm_sec;
  sprintf(minString, "%02d", ss);
  display.drawString(45, yOffset, minString);
  display.display();

  yOffset = 45;
  display.setFont(ArialMT_Plain_10);
  display.drawString(1, yOffset, itoa(hh, value, 10));
  display.drawString(12, yOffset, ":");

  display.drawString(15, yOffset, itoa(mm, value, 10));
  display.drawString(27, yOffset, ":");

  display.drawString(31, yOffset, itoa(ss, value, 10));
  display.display();

  char buffer[5];
  // sprintf String Print Formatted
  sprintf(buffer, "%02d", 4);
}

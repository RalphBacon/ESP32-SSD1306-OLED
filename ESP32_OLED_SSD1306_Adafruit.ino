#if(true) // includes
#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>
#endif

// My secret stuff (eg WiFi password)
#include "config.h"
// eg const *char password = "MySecretPassword"

// How we connect to your local wifi
#include <WiFi.h>

// See Arduino Playground for details of this useful time synchronisation library
#include <time.h>

// WiFi credentials
const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI PASSWORD";

// NTP server info
const char* ntpServer = "uk.pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

// WiFi specific defines
#define WifiTimeOutSeconds 10

// Days of week. Day 1 = Sunday
String DoW[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

// Months
String Months[] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// Initialize the OLED display
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

// These are used later
#define SDA_PIN 5
#define SCL_PIN 4

#if (true) // Forward declarations788k/9
void connectToWifi();
void printLocalTime();
void displayDateTime(struct tm timeinfo);
void displayDigits(int digits);
#endif

// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
void setup() {

	// Open serial communications and wait for port to open:
	Serial.begin(115200);

	// Set the I2C pins for this board
	display.I2Cpins(SDA_PIN, SCL_PIN);

	// Initialize OLED with the I2C addr 0x3C (for the 64x48)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	// Uncomment this next line if the processor is on the RIGHT!
	// It basically flips the display vertically
	display.setRotation(2);

	// Set initial screen parameters
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.clearDisplay();
	display.display();

	// Give user a chance to read display
	display.setTextColor(WHITE, BLACK);
	display.setCursor(0, 0);
	display.println("Setup is OK");
	display.display();

	// Connect to your local wifi (one time operation)
	connectToWifi();

	//init and get the time
	struct tm timeinfo;
	bool timeOK = false;
	while (!timeOK) {

		// Use the built in method to get the time
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

		// And set the daylight saving / timezone
		setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);

		if (!getLocalTime(&timeinfo)) {
			Serial.println("Failed to obtain time");
			delay(1000);
		}
		else {
			timeOK = true;
			time_t now = time(nullptr);
			Serial.println(ctime(&now));
		}
	}

	//disconnect WiFi as it's no longer needed
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);

	// Give user a chance to read display
	delay(2000);
}

// LOOP    LOOP    LOOP    LOOP    LOOP    LOOP    LOOP
void loop() {
	// Only do this every <period> seconds
	static unsigned long prevMillis = millis();

	// This just prints the "system time" every N seconds
	if (prevMillis + 100 < millis()) {
		printLocalTime();
		prevMillis = millis();
	}
}

// -----------------------------------------------------------------------
// Establish a WiFi connection with your router
//
// Note: if connection is established, and then lost for some reason, ESP
// will automatically reconnect to last used access point once it is again
// back on-line. This will be done automatically by WiFi library, without
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

	// Keep the display updated with wifi status (will auto reconnect if down)
	display.setCursor(0, 8);
	if (WiFi.status() != WL_CONNECTED) {
		display.setTextColor(BLACK, WHITE);
		display.println("WiFi   ZZZ");
	} else {
		display.setTextColor(WHITE, BLACK);
		display.println("WiFi    OK");
	}
	display.display();

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
	//Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

	// Update OLED
	displayDateTime(timeinfo);
}

// Display the date/time etc on the OLED
void displayDateTime(struct tm timeinfo) {

	// Keep the display updated with wifi status (will auto reconnect if down)
	display.setCursor(0, 8);
	if (WiFi.status() != WL_CONNECTED) {
		display.setTextColor(BLACK, WHITE);
		display.println("WiFi   BAD");
	} else {
		display.setTextColor(WHITE, BLACK);
		display.println("WiFi    OK");
	}
	display.display();

	// We'll grab the time so it doesn't change whilst we're printing it
	// Put the date/time info into a struct -> timeinfo;

	// DRY principle broken here
	bool timeOK = false;
	while (!timeOK) {
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);
		if (!getLocalTime(&timeinfo)) {
			Serial.println("Failed to obtain time");
			delay(1000);
		}
		else {
			timeOK = true;
		}
	}

	// Date
	display.setTextColor(WHITE, BLACK);
	display.setCursor(12, 32);
	display.print(DoW[timeinfo.tm_wday]);
	display.print(" ");
	displayDigits(timeinfo.tm_mday);
	display.print(" ");
	display.print(Months[timeinfo.tm_mon - 1]);

	// Time in BIGGER font
	display.setTextSize(2);
	display.setCursor(9, 41);
	displayDigits(timeinfo.tm_hour);
	display.print(":");
	displayDigits(timeinfo.tm_min);
	display.print(":");

	char buffer[5];
	// sprintf String Print Formatted
	sprintf(buffer, "%02d", timeinfo.tm_sec);
	display.print(buffer);
	display.display();
	display.setTextSize(1);
}

// There are MUCH better ways of ensuring a digit is two digits long
void displayDigits(int digits) {
	// utility for digital clock display: prints leading 0
	if (digits < 10)
		display.print('0');
	display.print(digits);
}

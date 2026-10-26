#ifdef ESP32
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#endif

#ifdef ARDUINO_ARCH_STM32
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#endif

#include <Arduino.h>

#if defined(ESP32) || defined(ARDUINO_ARCH_STM32)
#include "Clock.h"
#include "Display.h"
#endif
#include "Custom.h"
#include "Date.h"
#include "Geo.h"
#include "HebrewDate.h"

#define DISPLAY_SIZE 240
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4

Location location = Place::JERUSALEM.location();
Custom custom(Custom::LOCAL);

#ifdef __AVR__
Date currentDate(2026, 1, 1);
#endif

#if defined(ESP32) || defined(ARDUINO_ARCH_STM32)
Adafruit_ILI9341 panel(TFT_CS, TFT_DC, TFT_RST);
Display* display = nullptr;
StandardClock* standardClock = nullptr;
DateTime startDateTime;
unsigned long startMillis;
uint16_t frameBuffer[DISPLAY_SIZE * DISPLAY_SIZE];
#endif

void setup() {
#ifdef __AVR__
	Serial.begin(9600);
#endif

#if defined(ESP32) || defined(ARDUINO_ARCH_STM32)
	panel.begin();
	panel.setRotation(0);
	display = new Display(DISPLAY_SIZE);
	startDateTime = DateTime(Date(2026, 9, 15), Time(12, 0, 0));
	standardClock = new StandardClock(startDateTime, location, custom, DISPLAY_SIZE);
	startMillis = millis();
#endif
}

void loop() {
#ifdef __AVR__
	HebrewDate hebrewDate = HebrewDate::fromdate(currentDate);
	Serial.print(currentDate.year);
	Serial.print("-");
	Serial.print(currentDate.month);
	Serial.print("-");
	Serial.print(currentDate.day);
	Serial.print("  ");
	Serial.print(hebrewDate.year);
	Serial.print("-");
	Serial.print(hebrewDate.month);
	Serial.print("-");
	Serial.println(hebrewDate.day);
	currentDate = Date::fromordinal(currentDate.ordinal + 1);
	delay(100);
#endif

#if defined(ESP32) || defined(ARDUINO_ARCH_STM32)
	unsigned long elapsedMillis = millis() - startMillis;
	DateTime currentDateTime = startDateTime + (int)(elapsedMillis / 1000);
	standardClock->update(currentDateTime);
	standardClock->draw(*display);
	for (int index = 0; index < DISPLAY_SIZE * DISPLAY_SIZE; index++) {
		uint8_t red = display->buffer[index * 4 + 0];
		uint8_t green = display->buffer[index * 4 + 1];
		uint8_t blue = display->buffer[index * 4 + 2];
		frameBuffer[index] = panel.color565(red, green, blue);
	}
	panel.drawRGBBitmap(0, 0, frameBuffer, DISPLAY_SIZE, DISPLAY_SIZE);
#endif
}
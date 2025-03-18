#pragma once

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define PIN_SCL   1
#define PIN_SDA   2

float bpm = 0.0f;
int bpmAvg = 0;

MAX30105 hbSensor;

esp_err_t setupHB() {
  // Initialize sensor
  Serial.printf("Init HB sensor I2C...\n");
  if (!hbSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    return ESP_FAIL;
  }
  Serial.printf("HB sensor found!\n");

  Serial.printf("Init HB sensor...\n");
  hbSensor.setup(); // Configure sensor with default settings
  hbSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running

  Serial.printf("HB init successful!\n");
  return ESP_OK;
}

void detectHB(long irValue) {
  const byte RATE_SIZE = 8; // Increase this for more averaging. 4 is good.
  static byte rates[RATE_SIZE]; // Array of heart rates
  static int i = 0;
  static long lastBeat = 0; // Time at which the last beat occurred

  if (irValue < 50000) {
    bpm = -1.0f;
    bpmAvg = -1;
  }
  else if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    bpm = 60.0f / (delta / 1000.0f);

    if (bpm < 255 && bpm > 20) {
      rates[i++] = (byte)bpm; //Store this reading in the array
      i %= RATE_SIZE; //Wrap variable

      //Take average of readings
      bpmAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        bpmAvg += rates[x];
      bpmAvg /= RATE_SIZE;
    }
  }
}
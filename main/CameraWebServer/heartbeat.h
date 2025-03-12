#pragma once

#include "MAX30105.h"
#include "heartRate.h"

MAX30105 hbSensor;
float bpm = 0.0f;
int bpmAvg = 0;

void initHeartbeat() {

    // Initialize sensor
    if (!hbSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1)
            ;
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");

    hbSensor.setup();                    // Configure sensor with default settings
    hbSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
}

void detectHB()
{
    const byte RATE_SIZE = 8;     // Increase this for more averaging. 4 is good.
    static byte rates[RATE_SIZE]; // Array of heart rates
    static int i = 0;
    static long lastBeat = 0; // Time at which the last beat occurred

    long irValue = hbSensor.getIR();

    if (irValue < 50000)
    {
        bpm = -1.0f;
        bpmAvg = -1;
    }
    else if (checkForBeat(irValue) == true)
    {
        // We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        bpm = 60 / (delta / 1000.0);

        if (bpm < 255 && bpm > 20)
        {
            rates[i++] = (byte)bpm; // Store this reading in the array
            i %= RATE_SIZE;         // Wrap variable

            // Take average of readings
            bpmAvg = 0;
            for (byte x = 0; x < RATE_SIZE; x++)
                bpmAvg += rates[x];
            bpmAvg /= RATE_SIZE;
        }
    }
}

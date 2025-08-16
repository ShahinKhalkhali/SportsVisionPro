#pragma once
#include <Adafruit_MPU6050.h> // Replace with ICM20948
#include <Adafruit_Sensor.h>  // Replace with ICM20948 equivalent?

#define GYRO_DELAY (10)
#define TEMPERATURE_DELAY (1000)
#define ACCEL_DELAY (200)

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

// Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

void initMPU()
{
    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }
    Serial.println("MPU6050 Found!");
}

void updateGyroReadings()
{
    mpu.getEvent(&a, &g, &temp);

    float gyroX_temp = g.gyro.x;
    if (abs(gyroX_temp) > gyroXerror)
    {
        gyroX += gyroX_temp / 50.00;
    }

    float gyroY_temp = g.gyro.y;
    if (abs(gyroY_temp) > gyroYerror)
    {
        gyroY += gyroY_temp / 70.00;
    }

    float gyroZ_temp = g.gyro.z;
    if (abs(gyroZ_temp) > gyroZerror)
    {
        gyroZ += gyroZ_temp / 90.00;
    }
}

void updateAccReadings()
{
    mpu.getEvent(&a, &g, &temp);
    // Get current acceleration values
    accX = a.acceleration.x;
    accY = a.acceleration.y;
    accZ = a.acceleration.z;
}

void updateTempReadings()
{
    mpu.getEvent(&a, &g, &temp);
    temperature = temp.temperature;
}

void updateIMU() {
    static unsigned long lastTime = 0;
    static unsigned long lastTemperature = 0;
    static unsigned long lastAcc = 0;

    unsigned long time = millis();
    if ((time - lastTime) > GYRO_DELAY)
    {
        // Send Events to the Web Server with the Sensor Readings
        updateGyroReadings();
        lastTime = time;
    }
    if ((time - lastAcc) > ACCEL_DELAY)
    {
        // Send Events to the Web Server with the Sensor Readings
        updateAccReadings();
        lastAcc = time;
    }
    if ((time - lastTemperature) > TEMPERATURE_DELAY)
    {
        // Send Events to the Web Server with the Sensor Readings
        updateTempReadings();
        temperature = temperature - 20;
        lastTemperature = time;
    }
}
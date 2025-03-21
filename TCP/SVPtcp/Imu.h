#pragma once

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

float gyroXerror = 0.0f;
float gyroYerror = 0.0f;
float gyroZerror = 0.0f;

esp_err_t setupIMU(){
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    return ESP_FAIL;
  }
  Serial.printf("MPU6050 Found!\n");

  Serial.printf("Adjusting offsets...\n");

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  gyroXerror = g.gyro.x;
  gyroYerror = g.gyro.y;
  gyroZerror = g.gyro.z;

  return ESP_OK;
}

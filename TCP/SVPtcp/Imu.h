#pragma once

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

esp_err_t setupIMU(){
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    return ESP_FAIL;
  }
  Serial.printf("MPU6050 Found!\n");
  return ESP_OK;
}

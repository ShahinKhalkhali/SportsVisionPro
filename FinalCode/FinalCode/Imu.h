
#include <Wire.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

Adafruit_ICM20948 icm;



#define PIN_SCL 1
#define PIN_SDA 2

esp_err_t setupIMU() {
  // Initialize sensor
  Serial.printf("Init IMU sensor I2C...\n");
  if (!icm.begin_I2C(0x68)) {
    Serial.printf("ERROR: ICM20948 was not found. Please check wiring/power.\n");
    return ESP_FAIL;
  }
  Serial.printf("IMU sensor found!\n");
  return ESP_OK;
}


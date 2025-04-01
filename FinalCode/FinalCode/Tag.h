

esp_err_t setupTAG() {
  // Initialize sensor
  Serial.printf("Init UWB TAG SPI...\n");
  // if (!icm.begin_I2C(0x68)) {
  //   Serial.printf("ERROR: ICM20948 was not found. Please check wiring/power.\n");
  //   return ESP_FAIL;
  // }
  Serial.printf("UWB TAG found!\n");
  return ESP_OK;
}

#pragma once

#include <WiFi.h>

esp_err_t setupWifi(const char* ssid, const char* password) {
  Serial.printf("Connecting to %s", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int cnt{ 0 };
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("...\n");
    delay(1000);
    if (cnt++ >= 10) {
      return ESP_FAIL;
    }
  }
  Serial.printf("\nConnected!\n");
  return ESP_OK;
}

uint8_t connectToServer(WiFiClient* client, const char* server, uint16_t port) {
  static bool wasConnected{ false };
  if (wasConnected) {
    Serial.printf("Server disconnected!\n");
    wasConnected = false;
  }
  Serial.printf("Connecting to: %s:%d...\n", server, port);
  if (!client->connect(server, port)) {
    Serial.printf("Connection failed...\n");
  }
  else {
    Serial.printf("Connection success!\n");
    wasConnected = true;
  }
  return client->connected();
}
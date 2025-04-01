#include <WiFi.h>
#include <WiFiUdp.h>
#include "Camera.h"
#include "Heartbeat.h"
#include "Imu.h"

#define EN_CAM
#define EN_HB
#define EN_IMU
//////////////////////////////////////////////////////////////////////////////// ENABLE TAG HERE
// #define EN_TAG

#define DELAY_LOOP        (5000 / portTICK_PERIOD_MS)
#define DELAY_RESTART     (1000 / portTICK_PERIOD_MS)
#define DELAY_TASK_START  (100 / portTICK_PERIOD_MS)
#define DELAY_TASK_CAM    (20 / portTICK_PERIOD_MS)
#define DELAY_TASK_HB     (10 / portTICK_PERIOD_MS)
#define DELAY_TASK_IMU    (10 / portTICK_PERIOD_MS)
#define DELAY_TASK_TAG    (10 / portTICK_PERIOD_MS)
#define DELAY_SEND_MSG    (5 / portTICK_PERIOD_MS)

#define DELAY_SEND_HB     250
#define DELAY_SEND_IMU    250
#define DELAY_SEND_TAG    250

#define STACK_SIZE_CAM  8192
#define STACK_SIZE_HB   4096
#define STACK_SIZE_IMU  4096
#define STACK_SIZE_TAG  4096

#define PRI_TASK_CAM  2
#define PRI_TASK_HB   1
#define PRI_TASK_IMU  1
#define PRI_TASK_TAG  1

//////////////////////////////////////////////////////////////////////////////// SETUP THESE FOR CAPSTONE
const char* ssid = "BELL946";
const char* password = "A19E72DCCF47";
const char* server = "192.168.2.18";

// these should be ok, match python script tho!
const uint16_t port = 12000;
const int MAX_PACKET_SIZE = 1280;

enum MSG_TYPE : uint8_t {
  TEXT = 0,
  STREAM,
  BPM,
  IMU,
  TAG
};

SemaphoreHandle_t mtxI2C = xSemaphoreCreateMutex();
SemaphoreHandle_t mtxUDP = xSemaphoreCreateMutex();

esp_err_t setupWifi(const char* ssid, const char* password) {
  Serial.printf("> Connecting to %s", ssid);
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

void sendMsg(MSG_TYPE type, size_t len, uint8_t* data) {
  static WiFiUDP udp;
  
  xSemaphoreTake(mtxUDP, portMAX_DELAY);

  // show message
  Serial.printf("> Sending  TYPE=%d  BYTES=%d  ", type, len);
  if (type != STREAM) {
    for (int i = 0; i < len; ++i)
      Serial.printf("%c", data[i]);
  }
  Serial.printf("\n");

  // message header
  udp.beginPacket(server, port);
  udp.write((uint8_t*)&len, 4);
  udp.write(type);
  udp.endPacket();
  vTaskDelay(DELAY_SEND_MSG);
  
  // message data
  if (type == STREAM) {
    for (size_t i{ 0 }; i < len; i += MAX_PACKET_SIZE) {
      uint8_t arr[MAX_PACKET_SIZE];
      size_t packetSize = len - i;
      if (packetSize > MAX_PACKET_SIZE) packetSize = MAX_PACKET_SIZE;
      memcpy(arr, data + i, packetSize);
      udp.beginPacket(server, port);
      udp.write(arr, packetSize);
      udp.endPacket();
      vTaskDelay(DELAY_SEND_MSG);
    }
  }
  else {
    udp.beginPacket(server, port);
    udp.write(data, len);
    udp.endPacket();
    vTaskDelay(DELAY_SEND_MSG);
  }

  xSemaphoreGive(mtxUDP);
}

void cameraTask(void* params) {
  vTaskDelay(DELAY_TASK_START);
  Serial.printf("> Camera Task created on Core %d!\n", xPortGetCoreID());
  camera_fb_t* fb = NULL;
  
  while (WiFi.isConnected()) {
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    fb = esp_camera_fb_get();
    xSemaphoreGive(mtxI2C);

    sendMsg(STREAM, fb->len, fb->buf);
    
    esp_camera_fb_return(fb);
    vTaskDelay(DELAY_TASK_CAM);
    // yield();
  }

  Serial.printf("> Camera Task Terminated!\n");
  vTaskDelete(NULL);
}

void heartbeatTask(void* params) {
  vTaskDelay(DELAY_TASK_START);
  Serial.printf("> Heartbeat Task created on Core %d!\n", xPortGetCoreID());

  unsigned long lastMillis = 0;
  long irValue = 0;
  int bpmAvg = 0;
  String msg;

  while (WiFi.isConnected()) {
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    irValue = hbSensor.getIR();
    xSemaphoreGive(mtxI2C);

    detectHB(bpmAvg, irValue);

    if (millis() - lastMillis > DELAY_SEND_HB) {
      msg = String(irValue > 50000 ? bpmAvg : -1);
      sendMsg(BPM, msg.length(), (uint8_t*)msg.c_str());
      lastMillis = millis();
    }

    vTaskDelay(DELAY_TASK_HB);
    // yield();
  }

  Serial.printf("> Heartbeat Task Terminated!\n");
  vTaskDelete(NULL);
}

void imuTask(void* params) {
  vTaskDelay(DELAY_TASK_START);
  Serial.printf("> IMU Task created on Core %d!\n", xPortGetCoreID());

  unsigned long lastMillis = 0;
  sensors_event_t a, g, t, m; // acc gyro temp mag
  String msg;

  while (WiFi.isConnected()) {
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    icm.getEvent(&a, &g, &t, &m);
    xSemaphoreGive(mtxI2C);

    if (millis() - lastMillis > DELAY_SEND_IMU) {
      msg = String(a.acceleration.x)  + ", " +  String(a.acceleration.y)  + ", " + String(a.acceleration.z) + ", " +
            String(g.gyro.x)          + ", " + String(g.gyro.y)           + ", " + String(g.gyro.z)         + ", " +
            // String(m.magnetic.x)      + ", " + String(m.magnetic.y)       + ", " + String(m.magnetic.z)     + ", " +
            String(t.temperature); 
      sendMsg(IMU, msg.length(), (uint8_t*)msg.c_str());
      lastMillis = millis();
    }

    vTaskDelay(DELAY_TASK_IMU);
    // yield();
  }

  Serial.printf("> IMU Task Terminated!\n");
  vTaskDelete(NULL);
}

void tagTask(void* params) {
  vTaskDelay(DELAY_TASK_START);
  Serial.printf("> TAG Task created on Core %d!\n", xPortGetCoreID());

  unsigned long lastMillis = 0;
  //////////////////////////////////////////////////////////////////////////////// TAG data here
  // sensors_event_t a, g, t, m; // acc gyro temp mag
  String msg;

  while (WiFi.isConnected()) {
    //////////////////////////////////////////////////////////////////////////////// TAG uses SPI instead of I2C
    // xSemaphoreTake(mtxI2C, portMAX_DELAY);
    // icm.getEvent(&a, &g, &t, &m);
    // xSemaphoreGive(mtxI2C);

    if (millis() - lastMillis > DELAY_SEND_TAG) {
      //////////////////////////////////////////////////////////////////////////////// TAG MESSAGE HERE
      // msg = RANGES
      // sendMsg(TAG, msg.length(), (uint8_t*)msg.c_str());
      lastMillis = millis();
    }

    vTaskDelay(DELAY_TASK_TAG);
    // yield();
  }

  Serial.printf("> TAG Task Terminated!\n");
  vTaskDelete(NULL);
}

void setup() {
  esp_err_t err = ESP_OK;

  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.printf("> Power on! Core %d initializing...\n", xPortGetCoreID());

  Wire.begin(PIN_SDA, PIN_SCL);

  //////////////////////////////////////////////////////////////////////////////// INIT SPI FOR TAG
  // spiIint();

  #ifdef EN_CAM
  err = setupCamera();
  if (err != ESP_OK) {
    Serial.printf("ERROR: Camera setup failed with error 0x%x\n", err);
    while(1) delay(1000);
  }
  #endif

  #ifdef EN_HB
  err = setupHB();
  if (err != ESP_OK) {
    Serial.printf("Heartbeat Sensor setup failed!\n");
    while(1) delay(1000);
  }
  #endif

  #ifdef EN_IMU
  err = setupIMU();
  if (err != ESP_OK) {
    Serial.printf("IMU Sensor setup failed!\n");
    while(1) delay(1000);
  }
  #endif

  //////////////////////////////////////////////////////////////////////////////// INIT TAG
  #ifdef EN_TAG
  err = setupTAG();
  if (err != ESP_OK) {
    Serial.printf("TAG Sensor setup failed!\n");
    while(1) delay(1000);
  }
  #endif
}

void loop() {
  if (!WiFi.isConnected()) {
    esp_err_t err = setupWifi(ssid, password);

    if (err != ESP_OK) {
      Serial.printf("Could not connect to WiFi!\n");
      vTaskDelay(DELAY_RESTART);
      return;
    }

    #ifdef EN_CAM
    xTaskCreatePinnedToCore(cameraTask, "cameraTask", STACK_SIZE_CAM, NULL, PRI_TASK_CAM, NULL, 0);
    #endif
    #ifdef EN_HB
    xTaskCreatePinnedToCore(heartbeatTask, "heartbeatTask", STACK_SIZE_HB, NULL, PRI_TASK_HB, NULL, 1);
    #endif
    #ifdef EN_IMU
    xTaskCreatePinnedToCore(imuTask, "imuTask", STACK_SIZE_IMU, NULL, PRI_TASK_IMU, NULL, 1);
    #endif
    #ifdef EN_TAG
    xTaskCreatePinnedToCore(tagTask, "tagTask", STACK_SIZE_TAG, NULL, PRI_TASK_TAG, NULL, 1);
    #endif
  }

  String msg = "Hello server, this is esp! It is now: " + String(millis());
  sendMsg(TEXT, msg.length(), (uint8_t*)msg.c_str());

  vTaskDelay(DELAY_LOOP);
}
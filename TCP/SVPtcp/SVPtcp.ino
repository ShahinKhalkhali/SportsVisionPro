#include "TCPSocket.h"
#include "Camera.h"
#include "Heartbeat.h"
#include "Imu.h"

// #define VERBOSE

#define DELAY_MS_CAM  10
#define DELAY_MS_HB   10
#define DELAY_MS_IMU  100
#define DELAY_MS_LOOP 1000

#define TICKS(t) (t * portTICK_PERIOD_MS)

// const char* ssid    { "BELL946" };      // YOUR NETWORK SSID HERE
// const char* password{ "A19E72DCCF47" }; // YOUR NETWORK PASSWORD HERE
// const char* server  { "192.168.2.18" }; // PUT YOUR COMPUTER IP HERE (use 'ipconfig' in cmd prompt)

const char* ssid    { "SportsVisionPro" };      // YOUR NETWORK SSID HERE
const char* password{ "pass12345" }; // YOUR NETWORK PASSWORD HERE
const char* server  { "192.168.43.130" }; // PUT YOUR COMPUTER IP HERE (use 'ipconfig' in cmd prompt)

const uint16_t port { 12000 };          // MAKE SURE PORT MATCHES BIND PORT IN PYTHON SCRIPT

SemaphoreHandle_t mtxTCP = xSemaphoreCreateMutex();
SemaphoreHandle_t mtxI2C = xSemaphoreCreateMutex();

// add your message types here:
enum MSG_TYPE : uint8_t {
  TEXT = 0,
  STREAM,
  BPM,
  BPM_AVG,
  IMU
};

// send data function
void sendMsg(WiFiClient* pClient, size_t len, MSG_TYPE type, uint8_t* data) {
  xSemaphoreTake(mtxTCP, portMAX_DELAY);

  Serial.printf("Sending BYTES=%d TYPE=%d  ", len, type);
  const int N = type == STREAM ? 0 : len;
  for (int i{ 0 }; i < N; ++i)
    Serial.printf("%c", data[i]);

  Serial.printf("\n");
  pClient->write((uint8_t*)&len, 4);
  pClient->write(type);
  pClient->write(data, len);

  xSemaphoreGive(mtxTCP);
}

void cameraTask(void* params) {
  #ifdef VERBOSE
  Serial.printf("Camera Task Created!\n");
  #endif
  WiFiClient* pClient = (WiFiClient*)params;
  camera_fb_t *fb = NULL;

  while (pClient->connected()) {
  #ifdef VERBOSE
    Serial.printf("Update fb...\n");
  #endif
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    fb = esp_camera_fb_get();
    xSemaphoreGive(mtxI2C);

  #ifdef VERBOSE
    Serial.printf("Sending fb data...\n");
  #endif
    sendMsg(pClient, fb->len, STREAM, fb->buf);
    vTaskDelay(pdMS_TO_TICKS(10));
    esp_camera_fb_return(fb);

    vTaskDelay(TICKS(DELAY_MS_CAM));
    // delay(10);
    // yield();
  }

  #ifdef VERBOSE
  Serial.printf("Exit Camera Task, client disconnected!\n");
  #endif
  vTaskDelete(NULL);
}

void hbTask(void* params) {
  #ifdef VERBOSE
  Serial.printf("HB Task created!\n");
  #endif
  WiFiClient* pClient = (WiFiClient*)params;
  unsigned long lastMillis{ millis() };
  byte rates[RATE_SIZE];
  int i{ 0 };

  while (pClient->connected()) {
    #ifdef VERBOSE
    Serial.printf("Update HB...\n");
    #endif
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    long irValue = hbSensor.getIR();
    xSemaphoreGive(mtxI2C);
    
    #ifdef VERBOSE
    Serial.printf("Detecting HB...\n");
    #endif
    detectHB(irValue, rates, i);

    if (millis() - lastMillis > 1000) {
      #ifdef VERBOSE
      Serial.printf("Sending HB data...\n");
      #endif
      String msg;
      // msg = String(bpm); sendMsg(msg.length(), BPM, (uint8_t*)msg.c_str()); vTaskDelay(pdMS_TO_TICKS(10));
      msg = String(irValue >= 50000 ? bpmAvg : -1); sendMsg(pClient, msg.length(), BPM_AVG, (uint8_t*)msg.c_str()); vTaskDelay(pdMS_TO_TICKS(10));
      lastMillis = millis();
    }
    vTaskDelay(TICKS(DELAY_MS_HB));
    // delay(10);
    // yield();
  }

  #ifdef VERBOSE
  Serial.printf("Exit HB Task, client disconnected!\n");
  #endif
  vTaskDelete(NULL);
}

void imuTask(void* params) {
  #ifdef VERBOSE
  Serial.printf("IMU Task created!\n");
  #endif
  WiFiClient* pClient = (WiFiClient*)params;
  unsigned long lastSend = 0;
  float accX{ 0.0f }, accY{ 0.0f }, accZ{ 0.0f };
  float gyroX{ 0.0f }, gyroY{ 0.0f }, gyroZ{ 0.0f };
  float temp{ 0.0f };

  while (pClient->connected()) {
    #ifdef VERBOSE
    Serial.printf("Update IMU data\n");
    #endif
    // sensors_event_t a, g, t;

    // xSemaphoreTake(mtxI2C, portMAX_DELAY);
    // mpu.getEvent(&a, &g, &t);
    // xSemaphoreGive(mtxI2C);
    
    // accX = a.acceleration.x;
    // accY = a.acceleration.y;
    // accZ = a.acceleration.z;
    // gyroX = g.gyro.x;
    // gyroY = g.gyro.y;
    // gyroZ = g.gyro.z;
    // temp = t.temperature;
    // temp = temp - 20;

    if (millis() - lastSend > 1000) {
      #ifdef VERBOSE
      Serial.printf("Send IMU data\n");
      #endif
      // String msg;
      // msg = String(accX); sendMsg(pClient, msg.length(), ACC_X, (uint8_t*)msg.c_str());
      // msg = String(accY); sendMsg(pClient, msg.length(), ACC_Y, (uint8_t*)msg.c_str());
      // msg = String(accZ); sendMsg(pClient, msg.length(), ACC_Z, (uint8_t*)msg.c_str());
      // msg = String(gyroX); sendMsg(pClient, msg.length(), GYRO_X, (uint8_t*)msg.c_str());
      // msg = String(gyroY); sendMsg(pClient, msg.length(), GYRO_Y, (uint8_t*)msg.c_str());
      // msg = String(gyroZ); sendMsg(pClient, msg.length(), GYRO_Z, (uint8_t*)msg.c_str());
      // msg = String(temp); sendMsg(pClient, msg.length(), TEMP, (uint8_t*)msg.c_str());
      // lastSend = millis();
    }

    vTaskDelay(TICKS(DELAY_MS_IMU));
    // delay(10);
    // yield();
  }

  #ifdef VERBOSE
  Serial.printf("Exit IMU Task, client disconnected!\n");
  #endif
  vTaskDelete(NULL);
}

void setup() {
  esp_err_t err{ 0 };

  Serial.begin(115200);

  // setup I2C bus
  Serial.printf("Setting I2C pins...\n");
  Wire.setPins(PIN_SDA, PIN_SCL);

  // init camera
  err = setupCamera();
  if (err != ESP_OK) {
    Serial.printf("Camera setup failed with error 0x%x\n", err);
    return;
  }

  // init heart beat sensor
  err = setupHB();
  if (err != ESP_OK) {
    Serial.printf("Heartbeat Sensor setup failed!\n");
    return;
  }

  // init IMU
  err = setupIMU();
  if (err != ESP_OK) {
    Serial.printf("IMU  setup failed!\n");
    return;
  }

  // more inits here
  // ...

  // init wifi
  err = setupWifi(ssid, password);
  if (err != ESP_OK) {
    Serial.printf("Could not connect to server!\n");
    return;
  }
}

unsigned long lastSample{ 0 };
unsigned long lastSend{ 0 };
float accX{ 0.0f }, accY{ 0.0f }, accZ{ 0.0f };
float gyroX{ 0.0f }, gyroY{ 0.0f }, gyroZ{ 0.0f };
float temp{ 0.0f };

void loop() {
  static WiFiClient client;
  static unsigned long lastMsg = millis();

  // try to reconnect if disconnected
  if (!client.connected()) {
    if (!WiFi.isConnected()) {
      Serial.printf("WiFi Disconnected!!??\n");
    }

    bool connected = connectToServer(&client, server, port);
    if (connected) {
        xTaskCreatePinnedToCore(cameraTask, "cameraTask", 8192, (void*)&client, 2, NULL, 0);
        vTaskDelay(TICKS(500));
        xTaskCreatePinnedToCore(hbTask, "hbTask", 2048, (void*)&client, 1, NULL, 1);
        vTaskDelay(TICKS(500));
        // xTaskCreatePinnedToCore(imuTask, "imuTask", 2048, (void*)&client, 1, NULL, 0);
        vTaskDelay(TICKS(500));
        // xTaskCreatePinnedToCore(sensorsTask, "sensorsTask", 8192, (void*)&client, 1, NULL, 1);
    }
    vTaskDelay(TICKS(DELAY_MS_LOOP));
    return;
  }
  // get and send data when connected

  unsigned long time = millis();

  // send text data
  if (time - lastMsg > 5000) {
    String msg = "Hello computer, this is esp! It is now: " + String(millis());
    sendMsg(&client, msg.length(), TEXT, (uint8_t*)msg.c_str());
    lastMsg = time;
  }

  if (time - lastSample > 100) {
    #ifdef VERBOSE
    Serial.printf("Update IMU data\n");
    #endif

    sensors_event_t a, g, t;
    xSemaphoreTake(mtxI2C, portMAX_DELAY);
    mpu.getEvent(&a, &g, &t);
    xSemaphoreGive(mtxI2C);

    accX = a.acceleration.x;
    accY = a.acceleration.y;
    accZ = a.acceleration.z;
    gyroX = g.gyro.x;
    gyroY = g.gyro.y;
    gyroZ = g.gyro.z;
    temp = t.temperature;
    temp = temp - 20;

    lastSample = time;
  }
    
  if (time - lastSend > 1000) {
    #ifdef VERBOSE
    Serial.printf("Send IMU data\n");
    #endif
    String msg;
    msg = String(accX) + ", " + String(accY) + ", " + String(accZ) + ", " + 
      String(gyroX) + ", " + String(gyroY) + ", " + String(gyroZ) + ", "  +  String(temp);
    sendMsg(&client, msg.length(), IMU, (uint8_t*)msg.c_str());
    lastSend = time;
  }

  // send more data here
  // ...

  // experiment with changing delay size
  // according to cpu load
  vTaskDelay(TICKS(DELAY_MS_LOOP));
  // delay(10);
  // yield();
}

#include <Wire.h>
#include "webserver.h"
#include "camera.h"
#include "imu.h"
#include "heartbeat.h"

#define MODE_AP
// #define MODE_STA

#if defined(MODE_AP) && defined(MODE_STA)
  #error Please define either MODE_AP or MODE_STA
#elif defined(MODE_AP)
  #warning Compiling in MODE_AP
  #include <WiFiAP.h>
  const char *ssid = "SportsVisionPro";
  const char *password = "pass12345";
#elif defined(MODE_STA)
  #warning Compiling in MODE_STA
  const char *ssid = "SportsVisionPro"; //"--";
  const char *password = "pass12345"; // "--";
#else
  #error Please define either MODE_AP or MODE_STA
#endif


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  initHeartbeat();
  initMPU();
  initCamera();

#ifdef MODE_AP
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress ip = WiFi.softAPIP();
#else
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  IPAddress ip = WiFi.localIP();
#endif

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(ip);
  Serial.println("' to connect");
}

void loop() {

  // Update heartbeat data and broadcast via WebSocket
  detectHB();
  updateIMU();

  String data = "{\"bpm\": " + String(bpm) + ", \"avg_bpm\": " + String(bpmAvg) + ", \"ax\": " + String(accX) + ", \"ay\": " + String(accY) + ", \"az\": " + String(accZ) + ", \"gx\": " + String(gyroX) + ", \"gy\": " + String(gyroY) + ", \"gz\": " + String(gyroZ) + ", \"temp\": " + String(temperature) + " }";
  
  webSocket.broadcastTXT(data);  // Send heartbeat to all clients
  webSocket.loop();  // Process WebSocket events
  yield();
}
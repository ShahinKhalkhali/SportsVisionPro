#include "TCPSocket.h"
#include "Camera.h"

const char* ssid    { "BELL946" };      // YOUR NETWORK SSID HERE
const char* password{ "A19E72DCCF47" }; // YOUR NETWORK PASSWORD HERE
const char* server  { "192.168.2.18" }; // PUT YOUR COMPUTER IP HERE (use 'ipconfig' in cmd prompt)
const uint16_t port { 12000 };          // MAKE SURE PORT MATCHES BIND PORT IN PYTHON SCRIPT

// add your message types here:
enum MSG_TYPE : uint8_t {
  TEXT = 0,
  STREAM
};

// send data function
void sendMsg(WiFiClient* client, size_t len, MSG_TYPE type, uint8_t* data) {
  Serial.printf("Sending BYTES=%d TYPE=%d\n", len, type);
  client->write((uint8_t*)&len, 4);
  client->write(type);
  client->write(data, len);
}

void setup() {
  Serial.begin(115200);

  esp_err_t err{ 0 };

  // init camera
  err = setupCamera();
  if (err != ESP_OK) {
    Serial.printf("Camera setup failed with error 0x%x\n", err);
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

void loop() {
  static WiFiClient client;

  // try to reconnect if disconnected
  if (!client.connected()) {
    connectToServer(&client, server, port);
    delay(1000);
  }
  // get and send data when connected
  else {
    // send text data
    const String msg{ "Hello computer, this is esp! It is now: " + String(millis()) };
    sendMsg(&client, msg.length(), MSG_TYPE::TEXT, (uint8_t*)msg.c_str());

    // send JPEG data
   camera_fb_t *fb = NULL;
   fb = esp_camera_fb_get();
   sendMsg(&client, fb->len, MSG_TYPE::STREAM, fb->buf);
   esp_camera_fb_return(fb);

   // send more data here
   // ...
  }

  // experiment with changing delay size
  // according to cpu load
  delay(33);
}

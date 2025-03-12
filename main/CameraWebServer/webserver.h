#pragma once
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "esp_http_server.h"
#include "camera.h"


// webpage header (html.h)
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

const char *HTML_PAGE PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Sports Vision Pro</title>
  <style>
    body { text-align: center; font-family: Arial, sans-serif; }
    #heartbeat { font-size: 24px; margin-top: 10px; color: red; }
    #accel { font-size: 24px; margin-top: 10px; color: black; }
  </style>
</head>
<body>

  <h2>Sports Vision Pro</h2>
  <img id="camera" src="/stream" width="640" />

  <div id="heartbeat"> BPM: --, Avg BPM: --</div>
  <div id="accel"> Ax: --, Ay: --, Az: --</div>
  <div id="gyro"> Gx: --, Gy: --, Gz: --</div>
  <div id="temp"> Temp: --</div>


  <script>
    var socket = new WebSocket('ws://' + window.location.hostname + ':81/');

    socket.onopen = function() {
      console.log('WebSocket connected');
    };

    socket.onclose = function() {
      console.log('WebSocket disconnected');
    };

    socket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      document.getElementById('heartbeat').innerHTML = 'BPM: ' + data.bpm + ', Avg BPM: ' + data.avg_bpm;
      document.getElementById('accel').innerHTML = 'Ax: ' + data.ax + ', Ay: ' + data.ay + ', Az: ' + data.az;
      document.getElementById('gyro').innerHTML = 'Gx: ' + data.gx + ', Gy: ' + data.gy + ', Gz: ' + data.gz;
      document.getElementById('temp').innerHTML = 'Temp: ' + data.temp;
    };

    socket.onerror = function(error) {
      console.error('WebSocket Error: ' + error);
    };
  </script>

</body>
</html>
)rawliteral";

WebSocketsServer webSocket = WebSocketsServer(81);

httpd_handle_t stream_httpd = NULL;

esp_err_t page_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, HTML_PAGE, strlen(HTML_PAGE));
}

esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;

  esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  while (true)
  {
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK)
    {
      char buff[128];
      size_t len = snprintf(buff, 128, _STREAM_PART, fb->len, fb->timestamp.tv_sec, fb->timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, buff, len);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    }

    Serial.printf("MJPG:  %u B  %d us  %f fps\n", fb->len, 0, 0.0f);

    esp_camera_fb_return(fb);
    fb = NULL;

    if (res != ESP_OK)
    {
      Serial.println("Send frame failed");
      break;
    }

    yield();
  }

  return res;
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println("WebSocket connected");
    break;
  case WStype_DISCONNECTED:
    Serial.println("WebSocket disconnected");
    break;
  case WStype_TEXT:
    break;
  }
}

void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;
  config.max_open_sockets = 5;

  httpd_uri_t page_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = page_handler,
      .user_ctx = NULL};

  httpd_uri_t stream_uri = {
      .uri = "/stream",
      .method = HTTP_GET,
      .handler = stream_handler,
      .user_ctx = NULL};

  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &page_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); // Set WebSocket event handler
}
#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"

#define sensor_t esp_sensor_t

#include "MAX30105.h"
#include "heartRate.h"

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define MODE_AP
// #define MODE_STA

#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  -1
#define SIOD_GPIO_NUM   2
#define SIOC_GPIO_NUM   1

#define Y9_GPIO_NUM    16
#define Y8_GPIO_NUM    39
#define Y7_GPIO_NUM    15
#define Y6_GPIO_NUM    40
#define Y5_GPIO_NUM     7
#define Y4_GPIO_NUM    41
#define Y3_GPIO_NUM     6
#define Y2_GPIO_NUM    42
#define VSYNC_GPIO_NUM  4
#define HREF_GPIO_NUM   5
#define PCLK_GPIO_NUM  38

#if defined(MODE_AP) && defined(MODE_STA)
  #error Please define either MODE_AP or MODE_STA
#elif defined(MODE_AP)
  #warning Compiling in MODE_AP
  #include <WiFiAP.h>
  const char *ssid = "SportsVisionPro";
  const char *password = "pass12345";
#elif defined(MODE_STA)
  #warning Compiling in MODE_STA
  const char *ssid = "SportsVisionPro"; //"BELL946";
  const char *password = "pass12345"; // "A19E72DCCF47";
#else
  #error Please define either MODE_AP or MODE_STA
#endif

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

unsigned long lastTime = 0;  
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;

void initMPU(){
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
}

void updateGyroReadings(){
  mpu.getEvent(&a, &g, &temp);

  float gyroX_temp = g.gyro.x;
  if(abs(gyroX_temp) > gyroXerror)  {
    gyroX += gyroX_temp/50.00;
  }
  
  float gyroY_temp = g.gyro.y;
  if(abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp/70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if(abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp/90.00;
  }
}

void updateAccReadings() {
  mpu.getEvent(&a, &g, &temp);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
 
}

void updateTempReadings(){
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
}

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

MAX30105 hbSensor;

WebSocketsServer webSocket = WebSocketsServer(81);

httpd_handle_t stream_httpd = NULL;

float bpm = 0.0f;
int bpmAvg = 0;

esp_err_t page_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, HTML_PAGE, strlen(HTML_PAGE));
}

esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;

  esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK) {
      char buff[128];
      size_t len = snprintf(buff, 128, _STREAM_PART, fb->len, fb->timestamp.tv_sec, fb->timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, buff, len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    }

    Serial.printf("MJPG:  %u B  %d us  %f fps\n", fb->len, 0, 0.0f);

    esp_camera_fb_return(fb);
    fb = NULL;

    if (res != ESP_OK) {
      Serial.println("Send frame failed");
      break;
    }

    yield();
  }

  return res;
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
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

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;
  config.max_open_sockets = 5;

  httpd_uri_t page_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = page_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &page_uri);
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);  // Set WebSocket event handler
}

void detectHB() {
  const byte RATE_SIZE = 8; //Increase this for more averaging. 4 is good.
  static byte rates[RATE_SIZE]; //Array of heart rates
  static int i = 0;
  static long lastBeat = 0; //Time at which the last beat occurred

  long irValue = hbSensor.getIR();

  if (irValue < 50000) {
    bpm = -1.0f;
    bpmAvg = -1;
  }
  else if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    bpm = 60 / (delta / 1000.0);

    if (bpm < 255 && bpm > 20)
    {
      rates[i++] = (byte)bpm; //Store this reading in the array
      i %= RATE_SIZE; //Wrap variable

      //Take average of readings
      bpmAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        bpmAvg += rates[x];
      bpmAvg /= RATE_SIZE;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize sensor
  if (!hbSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  hbSensor.setup(); //Configure sensor with default settings
  hbSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running

  // initMPU();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 12000000;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 6;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

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

  // if ((millis() - lastTime) > gyroDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   updateGyroReadings();
  //   lastTime = millis();
  // }
  // if ((millis() - lastTimeAcc) > accelerometerDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   updateAccReadings();
  //   lastTimeAcc = millis();
  // }
  // if ((millis() - lastTimeTemperature) > temperatureDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   updateTempReadings();
  //   temperature = temperature - 20;
  //   lastTimeTemperature = millis();
  // }

  // String data = "{\"bpm\": " + String(bpm) + ", \"avg_bpm\": " + String(bpmAvg) + ", \"ax\": " + String(accX) + ", \"ay\": " + String(accY) + ", \"az\": " + String(accZ) + ", \"gx\": " + String(gyroX) + ", \"gy\": " + String(gyroY) + ", \"gz\": " + String(gyroZ) + ", \"temp\": " + String(temperature) + " }";
  String data = "{\"bpm\": " + String(bpm) + ", \"avg_bpm\": " + String(bpmAvg) + " };";

  webSocket.broadcastTXT(data);  // Send heartbeat to all clients
  webSocket.loop();  // Process WebSocket events
  yield();
}

#include <SPI.h>
#include <DW1000Ranging.h>
#include "link.h"

const uint8_t PIN_RST = 40;
const uint8_t PIN_IRQ = 39;
const uint8_t PIN_CLK = 21;
const uint8_t PIN_MISO = 14;
const uint8_t PIN_MOSI = 12;
const uint8_t PIN_SS = 13;

const long SEND_INTERVAL_MS = 200; // <-- Send data every 200 milliseconds

struct MyLink *uwb_data;
long runtime = 0;
String all_json = "";

void newRange();
void newDevice(DW1000Device *device);
void inactiveDevice(DW1000Device *device);

esp_err_t setupTAG() {
  // Initialize sensor
  Serial.printf("Init UWB TAG SPI...\n");
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  DW1000Ranging.startAsTag("7D:00:22:EA:82:60:3B:9C", DW1000.MODE_LONGDATA_RANGE_LOWPOWER);

  uwb_data = init_link();

  Serial.printf("UWB TAG Initialized!\n");
  return ESP_OK;
}

void newRange() {
    // Your existing code...
    char c[30];
    uint16_t shortAddress = DW1000Ranging.getDistantDevice()->getShortAddress();
    float range = DW1000Ranging.getDistantDevice()->getRange();
    float rxPower = DW1000Ranging.getDistantDevice()->getRXPower();

    Serial.print("from: "); Serial.print(shortAddress, HEX);
    Serial.print("\t Range: "); Serial.print(range); Serial.print(" m");
    Serial.print("\t RX power: "); Serial.print(rxPower); Serial.println(" dBm");

    fresh_link(uwb_data, shortAddress, range, rxPower);
}

void newDevice(DW1000Device* device) {
    // Your existing code...
    uint16_t shortAddress = device->getShortAddress();
    Serial.print("ranging init; 1 device added ! -> ");
    Serial.print(" short:"); Serial.println(shortAddress, HEX);

    add_link(uwb_data, shortAddress);
}

void inactiveDevice(DW1000Device* device) {
    // Your existing code...
    uint16_t shortAddress = device->getShortAddress();
    Serial.print("delete inactive device: ");
    Serial.println(shortAddress, HEX);

    delete_link(uwb_data, shortAddress);
}
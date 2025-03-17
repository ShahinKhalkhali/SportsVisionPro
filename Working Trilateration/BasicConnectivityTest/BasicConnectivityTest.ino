/*
 * MIT License
 * 
 * Copyright (c) 2018 Michele Biondi, Andrea Salvatori
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file BasicConnectivityTest.ino
 * Use this to test connectivity with your DW1000Ng from Arduino.
 * It performs an arbitrary setup of the chip and prints some information.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  - make real check of connection (e.g. change some values on DW1000Ng and verify)
 */

#include <DW1000Ng.hpp>

// // connection pins
// const uint8_t PIN_SCK = 18; // clock pin
// const uint8_t PIN_MISO = 19; // miso pin
// const uint8_t PIN_MOSI = 23; // mosi pin
// const uint8_t PIN_DWCS = 5; // select pin

// const uint8_t PIN_RST = 27; // reset pin
// const uint8_t PIN_IRQ = 26; // irq pin
// const uint8_t PIN_SS = 5; // spi select pin

// connection pins
const uint8_t PIN_SCK = 36; // clock pin
const uint8_t PIN_MISO = 37; // miso pin
const uint8_t PIN_MOSI = 35; // mosi pin
const uint8_t PIN_DWCS = 39;// select pin

const uint8_t PIN_RST = 2; // reset pin
const uint8_t PIN_IRQ = 4; // irq pin
const uint8_t PIN_SS = 39; // spi select pin

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  // initialize the driver
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_DWCS);
  DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST, SPI);
  Serial.println(F("DW1000Ng initialized ..."));
  // general configuration
  DW1000Ng::setDeviceAddress(5);
  DW1000Ng::setNetworkId(10);
  Serial.println(F("Committed configuration ..."));
  // wait a bit
  delay(1000);
}

void loop() {
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DW1000Ng::getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DW1000Ng::getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DW1000Ng::getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DW1000Ng::getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // wait a bit
  delay(10000);
}

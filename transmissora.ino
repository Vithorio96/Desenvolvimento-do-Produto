#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct DataPacket {
  float accelX;
  float accelY;
  float accelZ;
  float temperature;
};

DataPacket data;
uint8_t macAddress[6] = {0xC0, 0x5D, 0x89, 0xB0, 0x86, 0x18};
esp_now_peer_info_t peerInfo;
unsigned long lastAccelTime = 0;
unsigned long lastTempTime = 0;
const unsigned long accelInterval = 10;
const unsigned long tempInterval = 5000;
bool newTempAvailable = false;
float latestTemp = -1000;

void setup() {
  Serial.begin(9600);
  if (!accel.begin()) {
    Serial.println("Falha ao inicializar o ADXL345!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_2_G);
  accel.setDataRate(ADXL345_DATARATE_100_HZ);
  sensors.begin();
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Falha ao inicializar o ESP-NOW");
    return;
  }
  memcpy(peerInfo.peer_addr, macAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Falha ao adicionar peer");
    return;
  }
}

void sendData() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastAccelTime >= accelInterval) {
    sensors_event_t event;
    accel.getEvent(&event);

    data.accelX = event.acceleration.x;
    data.accelY = event.acceleration.y;
    data.accelZ = event.acceleration.z;
    if (newTempAvailable) {
      data.temperature = latestTemp;
      newTempAvailable = false;
    } else {
      data.temperature = -1000;
    }
    lastAccelTime = currentMillis;
    esp_err_t result = esp_now_send(macAddress, (uint8_t *)&data, sizeof(data));
    if (result == ESP_OK) {
      Serial.print("Dados enviados: ");
      Serial.print(data.accelX); Serial.print(", ");
      Serial.print(data.accelY); Serial.print(", ");
      Serial.print(data.accelZ); Serial.print(", ");
      Serial.println(data.temperature);
    } else {
      Serial.println("Falha ao enviar dados");
    }
  }
  if (currentMillis - lastTempTime >= tempInterval) {
    sensors.requestTemperatures();
    latestTemp = sensors.getTempCByIndex(0);
    newTempAvailable = true;
    lastTempTime = currentMillis;
  }
}

void loop() {
  sendData();
}
#include <esp_now.h>
#include <WiFi.h>
struct DataPacket {
  float accelX;
  float accelY;
  float accelZ;
  float temperature;
};
DataPacket receivedData;
void onDataReceived(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(receivedData)) {
    memcpy(&receivedData, incomingData, sizeof(receivedData));
    Serial.print("X: "); Serial.print(receivedData.accelX);
    Serial.print(", Y: "); Serial.print(receivedData.accelY);
    Serial.print(", Z: "); Serial.print(receivedData.accelZ);
    Serial.print(", Temp: ");
    if (receivedData.temperature > -100 && receivedData.temperature < 100) {
      Serial.println(receivedData.temperature);
    } else {
      Serial.println("N/A");
    }
  } else {
    Serial.println("Erro: tamanho de pacote incorreto.");
  }
}
void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Falha ao inicializar o ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataReceived);
}
void loop() {
  delay(10);
}
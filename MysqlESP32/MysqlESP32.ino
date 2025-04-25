#include <WiFi.h>
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

const char* ssid = "RT-2.4GHz_WiFi_2058";
const char* password = "Ud3mUbC3";
const char* ws_host = "192.168.0.15"; // IP сервера
const int ws_port = 8000;
const char* ws_path = "/ws";
bool clear = false;
void setup() {
  Serial.begin(115200);
  
  // Подключение к WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // Настройка WebSocket
  webSocket.begin(ws_host, ws_port, ws_path);
  webSocket.enableHeartbeat(15000, 3000, 2);  // Ping каждые 15 сек, timeout 3 сек, 2 попытки
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  
  static unsigned long last_send = 0;
  if(millis() - last_send > 5000) {
    sendSensorData(clear);
    clear = !clear;
    last_send = millis();
  }
}

void sendSensorData(bool clear_) {
  if(webSocket.isConnected()) {
    // Формируем JSON вручную
    String json = "{";
    json += "\"status\":1,";
    json += "\"level_garbage\":" + String(random(0,100)/10.0) + ",";
    json += "\"level_energy\":" + String(random(0,100));
    json += (clear_ == true) ? ",\"clear\":1" : "";
    json += "}";
    
    webSocket.sendTXT(json);
    Serial.println("Sent: " + json);
  }
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected");
      break;
    case WStype_TEXT:
      if(strcmp((char*)payload, "ping") == 0) {
        webSocket.sendTXT("pong");
      } else {
        Serial.printf("Received: %s\n", payload);
      }
      break;
  }
}
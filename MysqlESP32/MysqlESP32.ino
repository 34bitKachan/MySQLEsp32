#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "RT-2.4GHz_WiFi_2058";
const char* password = "Ud3mUbC3";
const char* ws_host = "192.168.0.15"; // IP сервера
const int ws_port = 5000;
const char* ws_path = "/api";
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
}

void loop() {
  
  static unsigned long last_send = 0;
  if(millis() - last_send > 5000) {
    sendSensorData(clear);
    clear = !clear;
    last_send = millis();
  }
}

void sendSensorData(bool clear_) {
    // Формируем JSON вручную
    String json = "{";
    json += "\"status\":1,";
    json += "\"level_garbage\":" + String(random(0,100)/10.0) + ",";
    json += "\"level_energy\":" + String(random(0,100));
    json += (clear_ == true) ? ",\"clear\":1" : "";
    json += "}";
    HTTPClient http;
    http.addHeader("Content-Type", "application/json");
    http.begin("http://" + ws_host + ":" + ws_port + ws_path);
    Serial.println("Sent: " + json);
    int httpCode = http.POST(json);
    
    if(httpCode == HTTP_CODE_OK) {
        Serial.println(http.getString());
    }
    http.end();
}


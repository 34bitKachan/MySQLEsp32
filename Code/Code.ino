// #include <Arduino.h>
// #include <Wire.h>
// #include <VL53L1X.h>
// #include <SPI.h>
// #include "nRF24L01.h"
// #include <RF24.h>
// #include <WiFi.h>
// #include <WebSocketsClient.h>

// WebSocketsClient webSocket;

// const char* ssid = "RT-2.4GHz_WiFi_2058";
// const char* password = "Ud3mUbC3";
// const char* ws_host = "192.168.0.13"; // IP сервера
// const int ws_port = 5000;
// const char* ws_path = "/websocket";

// long time_send_DB = 10000; // промежуток времени записи в бд 10с (миллисекунды)

// #define WSZ 2     // Размер области поля зрения
// #define ledpin 2 // Пин первого светодиода
// #define ledpinn 0 // Пин второго светодиода
// #define ledpinnn 15 // Пин третьего светодиода
// #define PIN_CE  4  // Номер пина, к которому подключен вывод CE радиомодуля
// #define PIN_CSN 5 // Номер пина, к которому подключен вывод CSN радиомодуля

// float lenght_clear_musor = 1000; // длина при которой считается что мусор убрали

// float old_dist = 0; // Предыдущее значение дистанции
// float target_distance = 1000; // Целевая дистанция в миллиметрах (1 метр)
// int current_distance_percentage = 0; // Процент текущей дистанции в виде целого числа

// VL53L1X sensor;
// RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

// const uint64_t pipeOut = 0xE9E8F0F0E1LL;

// //--------Данные------------------
// struct Signal 
// {
//   int distance_percentage; // Поле для процента заполненности в виде целого числа
// };

// Signal data;

// unsigned long previousMillis = 0; // Хранит время последней передачи
// const long interval = 1000; // Интервал передачи данных (1 секунда)

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println("Старт");

//   // Инициализация I2C и датчика
//   Wire.begin();
//   Wire.setClock(400000); // используем 400 kHz I2C
//   sensor.setTimeout(500);
//   sensor.setROISize(WSZ, WSZ);
//   if (!sensor.init())
//   {
//     Serial.println("Не удалось обнаружить и инициализировать сенсор!");
//     while (1);
//   }
//   sensor.setDistanceMode(VL53L1X::Long);
//   sensor.setMeasurementTimingBudget(50000);
//   sensor.startContinuous(50);

//   // Инициализация светодиодов
//   pinMode(ledpin, OUTPUT);
//   pinMode(ledpinn, OUTPUT); // Инициализируем второй светодиод
//   pinMode(ledpinnn, OUTPUT); // Инициализируем третий светодиод

//   // Инициализация модуля NRF24L01
//   radio.begin();  
//   radio.setChannel(5); // Обмен данными будет вестись на пятом канале (2,405 ГГц)
//   radio.setDataRate(RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
//   radio.setPALevel(RF24_PA_MIN); // Выбираем малую мощность передатчика (-6dBm)
//   radio.openWritingPipe(pipeOut); // Открываем трубу с уникальным ID

//   WiFi.begin(ssid, password);
//   while(WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nWiFi connected");
  
//   // Настройка WebSocket
//   webSocket.begin(ws_host, ws_port, ws_path);
//   webSocket.onEvent(webSocketEvent);
//   webSocket.setReconnectInterval(5000);

// }

// void loop()
// {
//   sensor.read();

//   // Получаем текущее расстояние
//   float current_distance = sensor.ranging_data.range_mm;

//   // Проверяем, корректно ли считано расстояние
//   if (sensor.ranging_data.range_status == VL53L1X::RangeValid) {
//     // Вычисляем процент текущего расстояния относительно целевого
//     if (target_distance > 0) {
//       current_distance_percentage = (int)(100 - (current_distance / target_distance) * 100);
//       // Ограничиваем значение от 0 до 100
//       if (current_distance_percentage < 0) {
//         current_distance_percentage = 0;
//       } else if (current_distance_percentage > 100) {
//         current_distance_percentage = 100;
//       }
//     } else {
//       current_distance_percentage = 0; // Защита от деления на ноль
//     }
//   } else {
//     current_distance_percentage = 0; // Если расстояние невалидно, устанавливаем 0
//   }

//   Serial.print("Расстояние: ");
//   Serial.print(current_distance);
//   Serial.print(" мм\tзаполненость: ");
//   Serial.print(current_distance_percentage);
//   Serial.println("%");

//   // Управление светодиодами
//   digitalWrite(ledpin, LOW);
//   digitalWrite(ledpinn, LOW);
//   digitalWrite(ledpinnn, LOW);

//   if (current_distance < 200) // Если расстояние меньше 200 мм
//   {
//     digitalWrite(ledpin, HIGH); // Включаем светодиод на пине 23
//   }
//   else if (current_distance < 500) // Если расстояние меньше 500 мм
//   {
//     digitalWrite(ledpinn, HIGH); // Включаем светодиод на пине 19
//   }
//   else if (current_distance < 1000) // Если расстояние меньше 1000 мм
//   {
//     digitalWrite(ledpinnn, HIGH); // Включаем светодиод на пине 18
//   }

//   // Проверяем, прошло ли 1 секунда
//   unsigned long currentMillis = millis();
//   if (currentMillis - previousMillis >= interval) {
//     previousMillis = currentMillis; // Сохраняем текущее время

//     // Заполняем данные для передачи
//     data.distance_percentage = current_distance_percentage; // Сохраняем процент заполненности как целое число
//     // Передача данных через NRF24L01
//     radio.write(&data, sizeof(Signal)); // Отправка данных
  
//   webSocket.loop();
  
//   static unsigned long last_send = 0;
//   if(millis() - last_send > time_send_DB) {
//     if(current_distance > old_dist && current_distance >= lenght_clear_musor)
//       sendSensorData(current_distance_percentage,true);
//     else sendSensorData(current_distance_percentage,false);
//     old_dist = current_distance;
//     last_send = millis();
//   }

//   }

//   old_dist = current_distance;
//   Serial.print("\tСтатус: ");
//   Serial.print(VL53L1X::rangeStatusToString(sensor.ranging_data.range_status));
//   Serial.print("\tПиковый сигнал: ");
//   Serial.print(sensor.ranging_data.peak_signal_count_rate_MCPS);
//   Serial.print("\tФоновый: ");
//   Serial.print(sensor.ranging_data.ambient_count_rate_MCPS);
//   Serial.println();
// }

// void sendSensorData(int level_garbage, bool clear) {
//   if(webSocket.isConnected()) {
//     // Формируем JSON вручную
//     String json = "{";
//     json += "\"status\":1,";
//     json += "\"level_garbage\":" + String(level_garbage) + ",";
//     json += "\"level_energy\":" + String(random(0,100));
//     json += (clear_ == true) ? ",\"clear\":1" : "";
//     json += "}";
    
//     webSocket.sendTXT(json);
//     Serial.println("Sent: " + json);
//   }
// }

// void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
//   switch(type) {
//     case WStype_DISCONNECTED:
//       Serial.println("Disconnected");
//       break;
//     case WStype_CONNECTED:
//       Serial.println("Connected");
//       break;
//     case WStype_TEXT:
//       Serial.printf("Received: %s\n", payload);
//       break;
//   }
// }
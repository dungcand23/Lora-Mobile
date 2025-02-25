#include <Arduino.h>
#define RXD2 16
#define TXD2 17
#include "DHT.h"
#define DHTPIN 25     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#include <DHT.h>
bool mode_lora = false;
const int relay3 = 32; // Chân GPIO điều khiển relay 3
const int relay4 = 33; // Chân GPIO điều khiển relay 4
int i,j;
void setup() {
  Serial.begin(9600); // Khởi động UART0 với baud rate 9600
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Khởi động UART2 với baud rate 115200, RX = GPIO16, TX = GPIO17
  dht.begin();
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  i=0;j=0;
}

void loop() {
  if(mode_lora){
    sendSensorData();
    mode_lora = false;
  } else{
    processSerial2Commands();
  }
}

void processSerial2Commands() {
  if (Serial2.available()) {
    String commandData = Serial2.readStringUntil('\n');
    // Tách các lệnh từ chuỗi nhận được
    int start = 0;
    int delimiterIndex;

    while ((delimiterIndex = commandData.indexOf(';', start)) != -1) {
      String command = commandData.substring(start, delimiterIndex);
      start = delimiterIndex + 1;

      // Xử lý từng lệnh
      if (command == "RELAY3_on") {
        digitalWrite(relay3, HIGH);
        Serial.println("RELAY  3 - on");
      } else if (command == "RELAY3_off") {
        digitalWrite(relay3, LOW);
        Serial.println("RELAY  3 - off");
      } else if (command == "RELAY4_on") {
        digitalWrite(relay4, HIGH);
        Serial.println("RELAY  4 - off");
      } else if (command == "RELAY4_off") {
        digitalWrite(relay4, LOW);
        Serial.println("RELAY  4 - off");
      }
    }
    mode_lora = true;
  }
}
void sendSensorData() {
  // Đọc dữ liệu từ cảm biến
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Tạo chuỗi dữ liệu tổng hợp
  String sensorData = "TEMP:" + String(temperature) + ";HUM:" + String(humidity) + ";";
  // i++;j++;
  // Gửi chuỗi dữ liệu qua Serial2
  Serial2.println(sensorData);

  // In dữ liệu ra Serial để kiểm tra
  Serial.print("Sensor data sent: ");
  Serial.println(sensorData);
}


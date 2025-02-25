#include <HardwareSerial.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Keypad.h>


HardwareSerial MySerial(2);
#include "DHT.h"
#define DHTPIN 25     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define RELAY_PIN_1  32 // the Arduino pin, which connects to the IN1 pin of relay module
#define RELAY_PIN_2  33 // the Arduino pin, which connects to the IN2 pin of relay module
bool mode_lora = false;
unsigned long previousMillis = 0;
const long interval = 10000;
unsigned long currentMillis;
// Cấu hình keypad 1x4
#define COL1 19
#define COL2 18
#define COL3 22
#define COL4 21
unsigned long lastPressTime[4] = {0, 0, 0, 0}; 
const unsigned long debounceDelay = 1000;  
// Chèn thông tin đăng nhập mạng của bạn
// #define WIFI_SSID "A07.03"
// #define WIFI_PASSWORD "tamsomot"
#define WIFI_SSID "Milano 2"
#define WIFI_PASSWORD "79797979"
// Cung cấp thông tin quy trình tạo mã thông báo.
#include <addons/TokenHelper.h>
// Cung cấp thông tin in tải trọng RTDB và các chức năng trợ giúp khác.
#include <addons/RTDBHelper.h>
// Chèn Khóa API của dự án Firebase
#define API_KEY "AIzaSyAu4ErrX0s_iULifq7pt1F-2FVOpFb70Fg"
#define DATABASE_URL "https://esp32-f1e53-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "dungcand123@gmail.com"
#define USER_PASSWORD "d2311200@"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String giomo[4], phutmo[4], giotat[4], phuttat[4];
int ngay, thang, gio, phut, giay;

String light, light1, light2, light21, light3, light4;
// String gio, phut = "";
String value;
String previous_light = "";
String previous_light1 = "";
String previous_light2 = "";
String previous_light21 = "";
unsigned long dataMillis = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
// byte ngay, thang, nam, giay;
float t, h;
int s;

String On = "on";
String Off = "off";
String formattedDate;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
String GT = "0";
String GTS = "0";
// Khai báo biến để lưu trữ trạng thái bật/tắt của từng thiết bị
int ena1 = 0, ena2 = 0, ena3 = 0, ena4 = 0;
int previous_ena1 = -1, previous_ena2 = -1, previous_ena3 = -1, previous_ena4 = -1;
// Biến để kiểm tra trạng thái chỉ chạy một lần
bool k1 = false, k2 = false, k3 = false, k4 = false;
bool ena = 0, k = 0;

boolean manual = false;  // đánh dấu trạng thái thủ công của đèn
bool manual1 = false;
bool manual2 = false;
bool manual3 = false;
bool manual4 = false;
bool receivedData = false;
int previous_ena = -1;
String inputString = "";    
boolean stringComplete = false;  // Biến cờ để kiểm tra xem chuỗi đã hoàn thành chưa

void setup() {
  Serial.begin(9600);
  MySerial.begin(9600, SERIAL_8N1, 16, 17);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Dang ket noi");
   timeClient.begin();
  timeClient.setTimeOffset(25200);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  dht.begin();
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1500);
  }
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  
  Serial.println ("");
  Serial.println ("Da ket noi WiFi!");
  Serial.println(WiFi.localIP());
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println();
  Serial.println("------------------------------------");
  Serial.println("Connected...");
  delay(1000);  
  pinMode(COL1, INPUT_PULLUP);
  pinMode(COL2, INPUT_PULLUP);
  pinMode(COL3, INPUT_PULLUP);
  pinMode(COL4, INPUT_PULLUP);

}

void loop() {
  keypress();
  set_dht11();
  keypress();
  updateTime();
  keypress();
  fetchDataFromFirebase();
  keypress();
  controlRelays();
  keypress();
  if(Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/den1")) light = fbdo.stringData();
  keypress();
  if(Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/den2")) light1 = fbdo.stringData();
  keypress();
  if(Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/den1")) light2 = fbdo.stringData();
  keypress();
  if(Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/den2")) light21 = fbdo.stringData();
  checkLightStatus();
  keypress();
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    Serial.println("CANH BAO DA 10s KHONG NHAN DU LIEU TU KHU VUC 2");
    previousMillis = currentMillis;
    mode_lora = true;
  }
  keypress();
  if(mode_lora) {
    sendCommand();
    previousMillis = millis();
  } else {
    Serial.println("NO LORA");
    processSensorData();
  }
  
}
void updateTime() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  
  ngay = ptm->tm_mday;
  thang = ptm->tm_mon + 1; // Tháng bắt đầu từ 0
  gio = timeClient.getHours();
  phut = timeClient.getMinutes();
  giay = timeClient.getSeconds();

}
void sendCommand() {
  String configData = "";
      if (light2 == "on") {
        configData += "RELAY3_" + light2 + ";";
        // Serial.print("Relay 3 command: ");
        // Serial.println(light2);
      } else {
        configData += "RELAY3_" + light2 + ";";
        // Serial.print("Relay 3 command: ");
        // Serial.println(light2);
      }
    
      if (light21 == "on") {
        configData += "RELAY4_" + light21 + ";";
        // Serial.print("Relay 4 command: ");
        // Serial.println(light21);
      } else {
        configData += "RELAY4_" + light21 + ";";
        // Serial.print("Relay 4 command: ");
        // Serial.println(light21);
      }
    
    MySerial.println(configData);
    Serial.print("Config data sent: ");
    Serial.println(configData);
    mode_lora = false;
}
void processSensorData() {
  if (MySerial.available()) {
    String data = MySerial.readStringUntil('\n');
    // Tách các cặp dữ liệu từ chuỗi nhận được
    int start = 0;
    int delimiterIndex;

    while ((delimiterIndex = data.indexOf(';', start)) != -1) {
      String pair = data.substring(start, delimiterIndex);
      start = delimiterIndex + 1;
      keypress();
      // Xử lý từng cặp dữ liệu
      if (pair.startsWith("TEMP:")) {
        float temperature = pair.substring(5).toFloat();
        Firebase.RTDB.setFloat(&fbdo, "/ESP32/khuvuc2/temp", temperature);
        Serial.print("Temperature: ");
        Serial.println(temperature);
      } else if (pair.startsWith("HUM:")) {
        float humidity = pair.substring(4).toFloat();
        Firebase.RTDB.setFloat(&fbdo, "/ESP32/khuvuc2/hum", humidity);
        Serial.print("Humidity: ");
        Serial.println(humidity);
      }
    }
    mode_lora = true;
    previousMillis = millis();
  }
}
void keypress() {
  unsigned long currentTime = millis(); // Lấy thời gian hiện tại

  // Kiểm tra phím 1 (COL1)
  if (digitalRead(COL1) == LOW && (currentTime - lastPressTime[0] > debounceDelay)) {
    Serial.println("Key 1 Pressed");
    lastPressTime[0] = currentTime;
    if (light == "off"){
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den1", "on");
    } else if(light == "on") {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den1", "off");
    }
    
  }

  // Kiểm tra phím 2 (COL2)
  if (digitalRead(COL2) == LOW && (currentTime - lastPressTime[1] > debounceDelay)) {
    Serial.println("Key 2 Pressed");
    lastPressTime[1] = currentTime;
    if (light1 == "off"){
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den2", "on");
    } else if(light1 == "on") {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den2", "off");
    }
  }

  // Kiểm tra phím 3 (COL3)
  if (digitalRead(COL3) == LOW && (currentTime - lastPressTime[2] > debounceDelay)) {
    Serial.println("Key 3 Pressed");
    lastPressTime[2] = currentTime;
    if (light2 == "off"){
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den1", "on");
    } else if(light2 == "on") {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den1", "off");
    }
  }

  // Kiểm tra phím 4 (COL4)
  if (digitalRead(COL4) == LOW && (currentTime - lastPressTime[3] > debounceDelay)) {
    Serial.println("Key 4 Pressed");
    lastPressTime[3] = currentTime;
    if (light21 == "off"){
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den2", "on");
    } else if(light21 == "on") {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den2", "off");
    }
  }
}
void fetchDataFromFirebase() {
  // Relay 1
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/giomo")) giomo[0] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/phutmo")) phutmo[0] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/giotat")) giotat[0] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/phuttat")) phuttat[0] = fbdo.stringData();

  // Relay 2
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/giomo1")) giomo[1] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/phutmo1")) phutmo[1] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/giotat1")) giotat[1] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc1/phuttat1")) phuttat[1] = fbdo.stringData();

  // Relay 3
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/giomo")) giomo[2] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/phutmo")) phutmo[2] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/giotat")) giotat[2] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/phuttat")) phuttat[2] = fbdo.stringData();

  // Relay 4
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/giomo1")) giomo[3] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/phutmo1")) phutmo[3] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/giotat1")) giotat[3] = fbdo.stringData();
  if (Firebase.RTDB.getString(&fbdo, "/ESP32/khuvuc2/phuttat1")) phuttat[3] = fbdo.stringData();
}
void controlRelays() {
    if (giomo[0].toInt() == gio && phutmo[0].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den1", "on");
      Serial.println("Relay 1 ON");
    }
    keypress();
    if (giotat[0].toInt() == gio && phuttat[0].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den1", "off");
      Serial.println("Relay 1 OFF");
    }
    keypress();
    if (giomo[1].toInt() == gio && phutmo[1].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den2", "on");
      Serial.println("Relay 2 ON");
    }
    keypress();
    if (giotat[1].toInt() == gio && phuttat[1].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc1/den2", "off");
      Serial.println("Relay 2 OFF");
    }

    keypress();

    if (giotat[2].toInt() == gio && phuttat[2].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den1", "off");
      Serial.println("Relay 3 OFF");
    }
    keypress();
    if (giomo[2].toInt() == gio && phutmo[2].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den1", "on");
      Serial.println("Relay 3 ON");
    }

    if (giotat[2].toInt() == gio && phuttat[2].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den2", "off");
      Serial.println("Relay 4 OFF");
    }
    keypress();
    if (giomo[2].toInt() == gio && phutmo[2].toInt() == phut) {
      Firebase.RTDB.setString(&fbdo, "/ESP32/khuvuc2/den2", "on");
      Serial.println("Relay 4 ON");
    }
}
void set_dht11(){
  float h = dht.readHumidity();    
  float t = dht.readTemperature(); 
  // Serial.print("KHU VUC 1 Nhiet do: ");
  // Serial.println(t);               //Xuất nhiệt độ
  // Serial.print("KHU VUC 1 Do am: ");
  // Serial.println(h);               //Xuất độ ẩm
  Firebase.RTDB.setInt(&fbdo, "/ESP32/khuvuc1/temp", t);
  Firebase.RTDB.setInt(&fbdo, "/ESP32/khuvuc1/hum", h);
}
void checkLightStatus() {
  if (light == "on") {
    digitalWrite(RELAY_PIN_1, HIGH);
    // Serial.println("Relay 1 ON");
  } else {
    digitalWrite(RELAY_PIN_1, LOW); // Kích mức 1 để tắt relay
    // Serial.println("Relay 1 OFF");
  }
  keypress();
  // Kiểm tra trạng thái của light1
  if (light1 == "on") {
    digitalWrite(RELAY_PIN_2, HIGH);
    // Serial.println("Relay 2 ON");
  } else {
    digitalWrite(RELAY_PIN_2, LOW); // Kích mức 1 để tắt relay
    // Serial.println("Relay 2 OFF");
  }
}

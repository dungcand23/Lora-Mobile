#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "time.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "LoRa_E32.h"
LoRa_E32 e32ttl(&Serial2); //  RX AUX M0 M1 (ESP32)
/* 1. Define the WiFi credentials */

#define WIFI_SSID "dung"
#define WIFI_PASSWORD "12345678"
// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino
/* 2. Define the API Key */
#define FIREBASE_AUTH "OvCGi8aRIST8iEv1Gd9rCxvuoJiKhcTuvhb5Ydkx"
/* 3. Define the RTDB URL */
#define FIREBASE_HOST "https://esp32-f1e53-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "dungcand123@gmail.com"
#define USER_PASSWORD "d2311200"
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
byte ngay, thang,nam ,giay;
unsigned long t = 0;
String GT = "0"; 
String GTS = "0";

int s;
bool ena= 0,k =0 ;
String turnOn = "on";
String turnOff = "off";
String formattedDate;
String giomo ("");
String giotat ("");
String phutmo ("");
String phuttat ("");
String light ("");
String gio, phut ="15";
boolean manual = false; // đánh dấu trạng thái thủ công của đèn
bool receivedData = false; 
int previous_ena = -1;
long t1, t2, t3,dv, chuc, tram, nghin;
String str = "";
int nhietdo1, doam1  ;
// -------------------------------------
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {
  Serial2.begin(115200);
  Serial.begin(115200);
  delay(1000);  
  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Dang ket noi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1500);
  }
  Serial.println ("");
  Serial.println ("Da ket noi WiFi!");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println();
  Serial.println("------------------------------------");
  Serial.println("Connected...");
  delay(1000);
  
  e32ttl.begin();
  Serial.println("Hi, I'm going to send message!");
  ResponseStructContainer c;
  c = e32ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  configuration.ADDL = 0x06;  // Addr   : 0-65535
  configuration.ADDH = 0x00;
  configuration.CHAN = 0x09;  // Channel: 0-31

  configuration.OPTION.fec = FEC_1_ON;
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
  configuration.OPTION.transmissionPower = POWER_20;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.SPED.airDataRate = AIR_DATA_RATE_100_96;
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;

  // Set configuration changed and set to not hold the configuration
  ResponseStatus rs = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
  printParameters(configuration);
  c.close();
  Serial.println();
  Serial.println("Start listening!");
    
}

void loop()
{ if (runEvery(2000)){
  if (e32ttl.available() > 1) {
      // read the String message
    ResponseContainer rc = e32ttl.receiveMessage();
    // Is something goes wrong print error
    if (rc.status.code!=1){
        rc.status.getResponseDescription();
    }else{
        // Print the data received
        String message = rc.data;
        
        Serial.println(rc.data);        
        xulydulieu(message);
      if (t1 == 1) {
      nhietdo1 = t2 / 10.0;
      doam1    = t3 / 10.0;    
      Serial.print("Nhiệt độ 1: "); Serial.println(nhietdo1);
      Serial.print("Độ ẩm 1   : "); Serial.println(doam1);    
      Serial.println();
        }
      }
    }
  Firebase.setInt(fbdo, "/ESP32/temp", nhietdo1);
  Firebase.setInt(fbdo, "/ESP32/hum", doam1);   
  Serial.println();
  }
    
  time();
  
  delay(1000);
  if(Firebase.getString(fbdo,"/ESP32/light")) light = fbdo.stringData();
  Serial.println(light);
if (light != "") { // kiểm tra xem đã nhận được giá trị từ Firebase chưa
  onLightChanged(light); // xử lý sự kiện thay đổi giá trị của biến light
}

if (!manual) { // nếu đèn không trong trạng thái thủ công
  if(gio == giomo) {
    if(phut == phutmo && k == 0) {
      //bat den
      ena = 1;
      k = 1;
    }
    if(phut != phutmo) k = 0;
  }
  if(gio == giotat) {
    if(phut == phuttat && k == 0) {
      //tat den
      ena = 0;
    }
  }
}  


  if (ena != previous_ena) { // so sánh giá trị hiện tại với giá trị trước đó
   
  if(ena == 1)
  {
    Serial.println("Bat den!");
    String input = "1" ;
    Firebase.setString(fbdo, "/ESP32/light", "On");
    ResponseStatus rs = e32ttl.sendBroadcastFixedMessage( 0x09, input);
    Serial.println(rs.getResponseDescription());
  }  
  else
  {
    Serial.println("tat den!");
    String input = "0" ;
    Firebase.setString(fbdo, "/ESP32/light", "Off");
    ResponseStatus rs = e32ttl.sendBroadcastFixedMessage( 0x09, input);
    Serial.println(rs.getResponseDescription());
    }  
     previous_ena = ena; // lưu trữ giá trị hiện tại cho lần so sánh tiếp theo
  }  

}
boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
void onLightChanged(String value) {
 if (value == "On") {
  manual = true; // bật trạng thái thủ công
  ena = 1; // bật đèn
} else if (value == "Off") {
  manual = true; // bật trạng thái thủ công
  ena = 0; // tắt đèn
}
manual = false; // tắt trạng thái thủ công
}
void xulydulieu(String mes) {
  str = (String)mes;
  s = str.indexOf("b") - str.indexOf("a");
  if (s == 2) {
  t1 = str[str.indexOf("a") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("a") + 1] - '0';
    dv = str[str.indexOf("a") + 2] - '0';
    t1 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("a") + 1] - '0';
    chuc = str[str.indexOf("a") + 2] - '0';
    dv = str[str.indexOf("a") + 3] - '0';
    t1 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("a") + 1] - '0';
    tram = str[str.indexOf("a") + 2] - '0';
    chuc = str[str.indexOf("a") + 3] - '0';
    dv = str[str.indexOf("a") + 4] - '0';
    t1 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }
  s = str.indexOf("c") - str.indexOf("b");
  if (s == 2) {
    t2 = str[str.indexOf("b") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("b") + 1] - '0';
    dv = str[str.indexOf("b") + 2] - '0';
    t2 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("b") + 1] - '0';
    chuc = str[str.indexOf("b") + 2] - '0';
    dv = str[str.indexOf("b") + 3] - '0';
    t2 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("b") + 1] - '0';
    tram = str[str.indexOf("b") + 2] - '0';
    chuc = str[str.indexOf("b") + 3] - '0';
    dv = str[str.indexOf("b") + 4] - '0';
    t2 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }
  s = str.indexOf("d") - str.indexOf("c");
  if (s == 2) {
    t3 = str[str.indexOf("c") + 1] - '0';
  }
  else if (s == 3) {
    chuc = str[str.indexOf("c") + 1] - '0';
    dv = str[str.indexOf("c") + 2] - '0';
    t3 = chuc * 10 + dv;
  }
  else if (s == 4) {
    tram = str[str.indexOf("c") + 1] - '0';
    chuc = str[str.indexOf("c") + 2] - '0';
    dv = str[str.indexOf("c") + 3] - '0';
    t3 = tram * 100 + chuc * 10 + dv;
  }
  else if (s == 5) {
    nghin = str[str.indexOf("c") + 1] - '0';
    tram = str[str.indexOf("c") + 2] - '0';
    chuc = str[str.indexOf("c") + 3] - '0';
    dv = str[str.indexOf("c") + 4] - '0';
    t3 = nghin * 1000 + tram * 100 + chuc * 10 + dv;
  }
}

#include "Arduino.h"
#include "LoRa_E32.h"
#include "DHT.h"
LoRa_E32 e32ttl(&Serial2); //  RX AUX M0 M1 (ESP32)
#define DHTPIN 33     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define tbi 1       //  Bộ phát số 1
const int RELAY_PIN = 32;// ESP32 pin GIOP16 connected to the IN pin of relay

String str = "";
int h, t, tt, hh;
boolean lora_tx = true; // ban đầu module ở chế độ phát dữ liệu
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const unsigned long interval = 2000;

 
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);
//The setup function is called once at startup of the sketch

void setup()
{ Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  dht.begin();
  delay(1000);
  
  e32ttl.begin();
  ResponseStructContainer c;
  c = e32ttl.getConfiguration();
  // Lấy con trỏ cấu hình trước tất cả các thao tác khác là rất quan trọng
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  configuration.ADDL = 0x07;  // Addr   : 0-65535
  configuration.ADDH = 0x00;
  configuration.CHAN = 0x09;  // Channel: 0-31

  configuration.OPTION.fec = FEC_1_ON;// Chuyển tiếp cấu hình công tắc sửa 
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;// Cấu hình chế độ 
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;//Cấu hình quản lý pull- 
  configuration.OPTION.transmissionPower = POWER_20;// cấu hình công suất truyền 
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;// Chờ thời gian đánh thức 
  configuration.SPED.airDataRate = AIR_DATA_RATE_100_96;// Cấu hình tốc độ dữ liệu không 
  configuration.SPED.uartBaudRate = UART_BPS_9600;// Tốc độ truyền thông
  configuration.SPED.uartParity = MODE_00_8N1;// bit chẵn lẻ

  // Đặt cấu hình đã thay đổi và đặt thành không giữ cấu hình 
  ResponseStatus rs = e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);
  printParameters(configuration);
  c.close();
  Serial.println();
  Serial.println("Start listening!");  

  h = dht.readHumidity();
  t = dht.readTemperature();
  hh  = h*10 ;
  tt  = t*10 ;  
}
 
// The loop function is called in an endless loop
void loop()
{
  if (lora_tx) {
  Serial.print("Temp : ");  Serial.print(t);
  Serial.print(" Hum  : "); Serial.print(h);
  Serial.println();
  String mess1 = 'a' + String(tbi) + 'b' + String(tt) + 'c' + String(hh) + 'd';
  Serial.println(mess1);
  ResponseStatus rs = e32ttl.sendFixedMessage(0, 6, 0x09, mess1);
  Serial.println(rs.getResponseDescription());
  delay(2000); 
  // Sau khi phát xong, đặt module ở chế độ nhận dữ liệu
    lora_tx = false;
  }
   
  if (e32ttl.available()  ) 
  {
    ResponseContainer rc = e32ttl.receiveMessage();
    // First of all get the data
    String message = rc.data;
    if (rc.status.code!=1)
    {
        Serial.println(rc.status.getResponseDescription());
    }
    else
    {
      // Print the data received
      Serial.println(rc.status.getResponseDescription());
      Serial.println(rc.data);
      
      
      // Xử lý dữ liệu nếu đã đủ 2s
      currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
      if (message == "1")
      {
      Serial.println("Bat den!");
      digitalWrite(RELAY_PIN, HIGH);
      }
      if(message == "0")
      {
      Serial.println("tat den!");
      digitalWrite(RELAY_PIN, LOW); 
        }
      }
    }
    delay(2000);  
  } 
  lora_tx = true;
}



void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN); Serial.print(" "); Serial.print(configuration.HEAD, DEC); Serial.print(" "); Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
  Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, HEX); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
  Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

  Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
  Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
  Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

  Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN); Serial.print(" "); Serial.print(moduleInformation.HEAD, DEC); Serial.print(" "); Serial.println(moduleInformation.HEAD, HEX);

  Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
  Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");

}




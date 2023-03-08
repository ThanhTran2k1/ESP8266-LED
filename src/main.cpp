#include <Arduino.h>
#include <ESP8266WiFi.h> // Thư viện WIFI
#include <PubSubClient.h> // Thư viện MQTT
#include <TaskScheduler.h> // Thư viện lập lịch
//khai báo thư viện
#include <DMDESP.h>
#include <fonts/Mono5x7.h>
#include <fonts/ElektronMart6x16.h>
#include <fonts/EMSans6x8.h>
#include <fonts/EMSansSP8x16.h>
#include <fonts/ElektronMart6x8.h>
#include <fonts/ElektronMart6x12.h> //  chưa tìm ra cách set _font qua MQTT, vẫn phải set font bằng fix cứng
#include <ArduinoJson.h>
#include <WiFiManager.h>

// Khai báo để quét LED P10
#define DISPLAYS_WIDE 2 //--> Số cột của tấm LED
#define DISPLAYS_HIGH 2 //--> Số hàng của tấm LED
// Khai báo WIDE = 1, HIGH = 1 tức là tấm LED 16x32;
// Khai báo WIDE = 2, HIGH = 1 tức là tấm LED 16x64;
// Khai báo WIDE = 2, HIGH = 2 tức là tấm LED 32x64;
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  //--> Number of Panels P10 used (Column, Row)
String message = "";
int speed_scroll = 50; //  tốc độ cuộn, tốc độ càng thấp cuộn càng nhanh.
WiFiManager wifiManager;

Scheduler runner; // tạo con trỏ để chạy Scheduler
// Callback methods prototypes
void setupWiFi();
void t2Callback();
void t3Callback(); 
void connectMQTTCallback();
// Tasks
Task t1(2000, TASK_ONCE, &setupWiFi, &runner, true);  //adding task to the chain on creation
Task t2(0.2, TASK_FOREVER, &t2Callback, &runner, true);  //adding task to the chain on creation
Task connectMQTT(2000, TASK_ONCE, &connectMQTTCallback, &runner, true);
//wifi
const char* ssid = "AIoT Tang 1"; // tên WIFI
const char* password = "aiot1234@"; // Mật khẩu WIFI
//mqtt server
const char* mqtt_server = "aiot-jsc1.ddns.net"; // Server MQTT
const uint16_t mqtt_port = 1889; // Nghe Port
const char* usernameMQTT = "bangled"; // Tên đăng nhập MQTT
const char* passMQTT = "bangled"; // Mật khẩu MQTT
//text Scrolling
static char *Text[]={"CONFIG-LED"}; // Nội dung lần đầu khi nạp cho chip. Đây là dòng chạy scroll,. Nội dung này sẽ bị thay đổi khi có 1 payload bắn qua MQTT
// text static
String text_static = "MODE";// Nội dung lần đầu khi nạp cho chip. Đây là dòng đứng yên
WiFiClient espClient;// Khai báo một Client
PubSubClient client(espClient); // Khai báo PubSub để nhận dữ liệu

//json
DynamicJsonDocument doc(1024); // 

void callback(char* topic, byte* payload, int length) 
{
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  int line = doc["line"];
  String content = doc["content"];
  
  Serial.print("Có tin nhắn mới từ topic: ");
  Serial.println(topic);
  Serial.print("Line: ");
  Serial.println(line);
  Serial.print("Content: ");
  Serial.println(content);
  if(line == 2) {
    // nếu line = 2 thì thay đổi giá trị của mảng Text[0] thành giá trị String content vừa nhận được.
    strcpy(Text[0],content.c_str());
    speed_scroll = doc["speed"];
  }
  if(line == 1) {
    text_static = content;
  }
  content.clear();
}
// Kết nối tới MQTT server
void connectMQTTCallback() {
  client.setServer(mqtt_server,mqtt_port);
}

// Kết nối WIFI. 
void setupWiFi() {
  Serial.println();
  Serial.print("Đang kết nối tới ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("..");
  }
  Serial.println("");
  Serial.println("Đã kết nối WiFi !");
  Serial.println("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
  connectMQTT.enable(); // sau khi kết nối WIFI thì kết nối MQTT để nhận dữ liệu
}

void t2Callback() {
  client.setCallback(callback); //  duy trì kết nối MQTT để nhận dữ liệu
}

// Nếu mất kết nối thì reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Đang kết nối tới MQTT.....");
    // Create a random client ID
    String clientId = "bangled";
    // Attempt to connect
    if (client.connect(clientId.c_str(),usernameMQTT,passMQTT)) {
      Serial.println("Đã kết nối !");
      // ... and resubscribe
      client.subscribe("text-content"); // topic cần subscribe
      // client.subscribe("static-text");
    } else { // nếu không kết nối thì kết nối lại sau 5s
      Serial.print("Kết nối MQTT thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 3s");
      // Đợi 3 giây trước khi kết nối lại
      delay(3000);
    }
  }
  Serial.println("Bắt đầu nhận nội dung từ MQTT !");
}


void Scrolling_Text(int y, uint8_t scrolling_speed) {
  static uint32_t pM;
  static int x;
  int width = Disp.width();
  Disp.setFont(ElektronMart6x12); // Chọn font để hiển thị.
  int fullScroll = Disp.textWidth(Text[0]) + width;
  if((millis() - pM) > scrolling_speed) { 
    pM = millis();
    if (x < fullScroll) {
      ++x;
    } else {
      x = 0;
      return;
    }
    Disp.drawText(width - x, y, Text[0]);
  }  
}

void setup () {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Start");
  Disp.start(); // Khởi tạo hàm hiển thị
  Disp.setBrightness(100); // Thay đổi độ sáng 
  runner.startNow();  // Khởi tạo bộ lên lịch Scheduler và chạy bằng con trỏ runner
}


void loop () {
  
  runner.execute(); // Xử lý bộ lên lịch
  if(!client.connected()) {
    reconnect();
  } // Nếu mất kết nối thì tự động kết nối lại
  client.loop(); // Giữ kết nối.
  Disp.loop(); // Bắt buộc phải có hàm này trong loop để quét LED hiển thị
  Disp.drawText(0, 0, text_static); //Disp.drawText(vị trí x, vị trí y, String cần hiển thị)
  // Disp.setFont(ElektronMart6x16);
  Scrolling_Text(16, speed_scroll); //Chữ chạy Scrolling_Text (vị trí y, tốc độ cuộn)
  // Disp.drawChar(0,0,'------');

}
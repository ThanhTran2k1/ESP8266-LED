#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h> // Thư viện WIFI
// Cac thu vien de smart config WIFI
#include <ESP8266HTTPClient.h> // Thư viện để smart config WIFI
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
//
#include <PubSubClient.h> // Thư viện MQTT
#include <TaskScheduler.h> // Thư viện lập lịch
//khai báo thư viện quet led
#include <DMDESP.h>
#include <fonts/Mono5x7.h>
#include <fonts/ElektronMart6x16.h>
#include <fonts/EMSans6x8.h>
#include <fonts/EMSansSP8x16.h>
#include <fonts/ElektronMart6x8.h>
#include <fonts/ElektronMart6x12.h> //  chưa tìm ra cách set _font qua MQTT, vẫn phải set font bằng fix cứng


// Smart Config WiFi
int statusCode;
const char* ssidWF = "text";
const char* passWF = "text";
String st;
String content;

// Khai bao cac ham
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void configWifi(void);
void readEEPROM(void);
// Thiet lap server Local tai cong 80 moi khi duoc yeu cau
ESP8266WebServer server(80);

// Khai báo để quét LED P10
#define DISPLAYS_WIDE 2 //--> Số cột của tấm LED
#define DISPLAYS_HIGH 2 //--> Số hàng của tấm LED
// Khai báo WIDE = 1, HIGH = 1 tức là tấm LED 16x32;
// Khai báo WIDE = 2, HIGH = 1 tức là tấm LED 16x64;
// Khai báo WIDE = 2, HIGH = 2 tức là tấm LED 32x64;
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  //--> Number of Panels P10 used (Column, Row)
String message = "";
int speed_scroll = 50; //  tốc độ cuộn, tốc độ càng thấp cuộn càng nhanh.
// WiFiManager wifiManager;

Scheduler runner; // tạo con trỏ để chạy Scheduler
// Callback methods prototypes

void connectionWiFi(); 
void connectMQTTCallback();
void keepAliveCallback();
void t2Callback();
// Tasks
Task task_connectWifi(2000, TASK_ONCE, &connectionWiFi, &runner, true);  //adding task to the chain on creation
Task task_connectMQTT(2000, TASK_ONCE, &connectMQTTCallback, &runner, true);
// Task t2(0.2, TASK_FOREVER, &t2Callback, &runner, true);  //adding task to the chain on creation
//wifi
// const char* ssid = "AIoT Tang 1"; // tên WIFI
// const char* password = "aiot1234@"; // Mật khẩu WIFI
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
  client.setCallback(callback); //  duy trì kết nối MQTT để nhận dữ liệu

}

bool testWifi(void) {

  int count = 0;
  Serial.println("Doi wifi de ket noi");
  while(count<20) {
    if(WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print("*");
    count++;
  }
  Serial.println("Ket noi WiFi that bai !");
  return false;
}

void readEEPROM() {
  Serial.println("Dang doc SSID va PASS tu EEPROM ");
  String esid;
  for(int i=0;i<32;i++) {
    esid += char(EEPROM.read(i));
  }
  String epass = "";
  for(int i=32;i<96;i++) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.print("PASS: ");
  Serial.println(epass);
  ssidWF = esid.c_str();
  passWF = epass.c_str();
  WiFi.begin(ssidWF,passWF);
  
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
 
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("ESP8266-WiFi-Config", "12345678");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

// Kết nối WIFI. 
void connectionWiFi() {
  readEEPROM();
  if(testWifi()) {
    Serial.println("Ket noi WiFi thanh cong !");
    task_connectMQTT.enable(); // sau khi kết nối WIFI thì kết nối MQTT để nhận dữ liệu
  }else {
    Serial.println("bat che do HostPot");
    launchWeb();
    setupAP();
  }
  Serial.println();
  Serial.println("Waiting.");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
    server.handleClient();
  }
  
}

void createWebServer()
{
 {
    server.on("/", []() {
 
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
 
      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });
 
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
 
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
 
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
 
    });
  } 
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void t2Callback() {
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
  Serial.println("Ngat ket noi WiFi hien tai !");
  WiFi.disconnect();
  EEPROM.begin(512); // khoi tao EEPROM de luu WIFI
  Disp.start(); // Khởi tạo hàm hiển thị
  Disp.setBrightness(100); // Thay đổi độ sáng 
  // runner.addTask(task_connectWifi);
  runner.startNow();  // Khởi tạo bộ lên lịch Scheduler và chạy bằng con trỏ runner
}


void loop () {
  
  runner.execute(); // Xử lý bộ lên lịch
  if(!client.connected()) {
    reconnect();
  } // Nếu mất kết nối thì tự động kết nối lại
  client.loop(); // Giữ kết nối.
  Disp.loop(); // Bắt buộc phải có hàm này trong loop để quét LED hiển thị
  Disp.drawText(0, 1, text_static); //Disp.drawText(vị trí x, vị trí y, String cần hiển thị)
  // Disp.setFont(ElektronMart6x16);
  Scrolling_Text(17, speed_scroll); //Chữ chạy Scrolling_Text (vị trí y, tốc độ cuộn)
  // Disp.drawChar(0,0,'------');

}
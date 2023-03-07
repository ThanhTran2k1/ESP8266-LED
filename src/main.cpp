#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TaskScheduler.h>
//khai báo thư viện
#include <DMDESP.h>
#include <fonts/Mono5x7.h>

/**
A	D0
B	D6
CLK	D5
SCK	D3
R	D7
NOE	D8
GND	GND
*/
// Khai báo để quét LED P10
#define DISPLAYS_WIDE 1 //--> Panel Columns
#define DISPLAYS_HIGH 1 //--> Panel Rows
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);  //--> Number of Panels P10 used (Column, Row)
String message = "";


Scheduler runner;
// Callback methods prototypes
void setupWiFi();
void t2Callback();
void t3Callback(); 
void connectMQTTCallback();
// Tasks
Task t4();
Task t1(2000, TASK_ONCE, &setupWiFi, &runner, true);  //adding task to the chain on creation
Task t2(0.2, TASK_FOREVER, &t2Callback, &runner, true);  //adding task to the chain on creation
Task t3(5000, TASK_FOREVER, &t3Callback);
Task connectMQTT(2000, TASK_ONCE, &connectMQTTCallback, &runner, true);
//wifi
const char* ssid = "AIoT JSC";
const char* password = "aiot1234@";
//mqtt server
const char* mqtt_server = "192.168.1.133";
const uint16_t mqtt_port = 1883;
//text Scrolling
static char *Text[]={"12345"};

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, int length) 
{
  Serial.print("Co tin nhan moi tu topic: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);   
    message += (char)payload[i];
  }
  const char *msg = message.c_str();
  Text[0] = (char*)msg;  
  Serial.println();
  Serial.println(message);
  message.clear();
}

void connectMQTTCallback() {
  client.setServer(mqtt_server,mqtt_port);
  
}


void setupWiFi() {
  Serial.println();
  Serial.print("Đang kết nối tới ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  connectMQTT.enable();
}

void t2Callback() {
  client.setCallback(callback);
  // message = "";
  //  Disp.loop(); //--> Run "Disp.loop" to refresh the LED
  
}

void t3Callback() {
    Serial.print("t3: ");
    Serial.println(millis());
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Đang kết nối tới MQTT.....");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Đã kết nối!");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("scrolling-text");
      client.subscribe("static-text");
    } else {
      Serial.print("Kết nối MQTT thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5s");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  Serial.println("Da ket noi MQTT thanh cong !");
}

// void Scrolling_String(int y, String str, uint8_t scrolling_speed) {
//   static uint32_t pM;
//   static uint32_t x;
//   int width = Disp.width();
//   Disp.setFont(Mono5x7);
//   int fullScroll = Disp.textWidth(&str[0]) + width;
//   if((millis() - pM) > scrolling_speed) { 
//     pM = millis();
//     if (x < fullScroll) {
//       ++x;
//     } else {
//       x = 0;
//       return;
//     }
//     Disp.drawText(width - x, y, &str[0]);
//   }
// }


void Scrolling_Text(int y, uint8_t scrolling_speed) {
  static uint32_t pM;
  static int x;
  int width = Disp.width();
  Disp.setFont(Mono5x7);
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
  Serial.println("Scheduler TEST");
  Disp.start(); //--> Run the DMDESP library
  Disp.setBrightness(100); //--> Brightness level
  Disp.setFont(Mono5x7); //--> Determine the font used
  runner.startNow();  // set point-in-time for scheduling start
}


void loop () {
  
  runner.execute();
  if(!client.connected()) {
    reconnect();
  }
  client.loop();
  Disp.loop(); //--> Run "Disp.loop" to refresh the LED
  Disp.drawText(0, 0, "12345678910"); //--> Display text "Disp.drawText(x position, y position, text)"
  Scrolling_Text(8, 60); //--> Show running text "Scrolling_Text(y position, speed);"
//  Serial.println("Loop ticks at: ");
//  Serial.println(millis());
}
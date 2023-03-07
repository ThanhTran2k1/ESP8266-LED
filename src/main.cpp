#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TaskScheduler.h>
#include <DMDESP.h>
Scheduler runner;
// Callback methods prototypes
void setupWiFi();
void t2Callback();
void t3Callback(); 
void connectMQTTCallback();
// Tasks
Task t4();
Task t1(2000, TASK_ONCE, &setupWiFi, &runner, true);  //adding task to the chain on creation
Task t2(2000, TASK_FOREVER, &t2Callback, &runner, true);  //adding task to the chain on creation
Task t3(5000, TASK_FOREVER, &t3Callback);
Task connectMQTT(2000, TASK_ONCE, &connectMQTTCallback, &runner, true);
//wifi
const char* ssid = "AIoT JSC";
const char* password = "aiot1234@";
//mqtt server
const char* mqtt_server = "192.168.1.133";
const uint16_t mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, int length) 
{
  Serial.print("Co tin nhan moi tu topic:");
  Serial.println(topic);
  for (int i = 0; i < length; i++) 
  Serial.print((char)payload[i]);
  Serial.println();
}

void connectMQTTCallback() {
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);
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
      Serial.print("t2: ");
      Serial.println(millis());
  
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
      client.subscribe("inTopic");
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


void setup () {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Scheduler TEST");
  
  runner.startNow();  // set point-in-time for scheduling start
}


void loop () {
  
  runner.execute();
  if(!client.connected()) {
    reconnect();
  }
  client.loop();
//  Serial.println("Loop ticks at: ");
//  Serial.println(millis());
}
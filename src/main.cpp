#include <Arduino.h>
#include <RoninDMD.h>   
#include <PubSubClient.h>    
#include <ESP8266WiFi.h>             
#include <fonts/Arial_Black_16.h>// Include lib & font 
#include <fonts/SystemFont5x7.h>
#define FONT System5x7

#define WIDTH 1                          // Set width & height
#define HEIGHT 1
RoninDMD P10(WIDTH, HEIGHT);

const char* ssid = "AIoT JSC";
const char* password = "aiot1234@";
const char* mqtt_server = "192.168.1.140";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


String Message = "11111111";
void Scrolling_text(int text_height , int scroll_speed , String scroll_text ) {
  static uint32_t pM ;
  pM = millis();
  static uint32_t x = 0;
  scroll_text = scroll_text + " ";

  bool  scrl_while = 1 ;
  int dsp_width = P10.width();
  int txt_width = P10.textWidth(scroll_text);

  while (scrl_while == 1) {

    P10.loop();
    delay(1);
    if (millis() - pM > scroll_speed) {
      P10.setFont(FONT);
      P10.drawText(dsp_width - x, text_height, scroll_text);


      x++;
      if (x >  txt_width + dsp_width) {

        x = 0 ;
        scrl_while = 0 ;

      }
      pM = millis();

    }


  }

}
void setup() {
  Serial.begin(115200);
  P10.begin();              // Begin the display & font
  P10.setFont(FONT);
  P10.setBrightness(50);    // Set the brightness
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // P10.drawText(0 , 0, " :) "); // P10.drawText(position x , position y, String type text);

}

void loop() {

  P10.loop();          // Run DMD loop

  if (Serial.available() > 0) {        // Save message from serial
    Message =  Serial.readString();
  }

  Scrolling_text(0 , 50 , Message ); // Call the function to write scrolling text on screen.
                                     // like -> Scrolling_text( position y , scroll speed, String type text);
                                     // or for not scroll -> P10.drawText(position x , position y, String type text);

  

}



#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <PubSubClient.h>
#include <Ticker.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define WIFI 0
#define MQTT 1
#define WATER 2

// WiFi configuration
const char* ssid = "SAYANI_WIFI";
const char* password = "00011101";

// MQTT configuration
const char* mqtt_server = "192.168.8.10";

// OLED
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE); 

Ticker level;
Ticker ago;
WiFiClient espClient;
PubSubClient client(espClient);

String water_level_percentage = "--";
char display_buffer[16];
unsigned long mqtt_last_message_millis = 0;

// Application setup
void setup(void) {
  Serial.begin(115200);
  u8g2.begin();

  level.attach(5, display_level);
  ago.attach(10, display_ago);
  
  setup_wifi();
  setup_mqtt();
}

// Main loop
void loop(void) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    delay(500);
}

// Connect to WiFi
void setup_wifi() {
 
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Initialize MQTT
void setup_mqtt() {
  randomSeed(micros());
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// (Re)connect to MQTT
void reconnect() {
  if(WiFi.status() == WL_CONNECTED) {
       
    Serial.print("Attempting MQTT connection...");
    
     // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) { // This blocks the thread
      Serial.println("connected");
      client.subscribe("home/terrace/tank/water-level");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

// MQTT message handler
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  water_level_percentage = msg;
  mqtt_last_message_millis = millis();
}

void display() {
  u8g2.clearBuffer();
  display_ago();
  u8g2.sendBuffer();
}

void display_level() {
  water_level_percentage.toCharArray(display_buffer, 8);
  Serial.println(display_buffer);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso26_tr); 
  u8g2.drawStr(0,26,display_buffer); 
  u8g2.sendBuffer();
}

void display_ago() {
  if (mqtt_last_message_millis == 0) return;
  unsigned long current_millis = millis();
  int secs = (current_millis - mqtt_last_message_millis) / 1000;
  int mins = max(60, secs) / 60;

  if(mins >= 1440) {
    sprintf(display_buffer,"%d %s %s", (mins / 1440), "day", "ago"); 
  } else if(mins >= 60) {
    sprintf(display_buffer,"%d %s %s", (mins / 60), "hr", "ago"); 
  } else {
    sprintf(display_buffer,"%d %s %s", mins, "min", "ago"); 
  }
  
  Serial.println(display_buffer);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso16_tr);
  u8g2.drawStr(0,18,display_buffer);
  u8g2.sendBuffer();
}

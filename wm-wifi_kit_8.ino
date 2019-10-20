#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <PubSubClient.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define WIFI 0
#define MQTT 1
#define WATER 2

const char* ssid = "SAYANI_IOT";//"VIVA-Router-ADV-LTE";
const char* password = "00001111";//"VIVA770319";
const char* mqtt_server = "192.168.8.10";

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;
signed int reading = -99;
char msg[5];

void draw_symbol(uint8_t symbol, uint8_t color) {
  switch(symbol) {
    case WIFI:
      u8g2.setFontMode(1);
      u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(0, 12, 248);
      break;
    case MQTT:
      u8g2.setFontMode(1);
      u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(0, 12+2+12, 206);
      break;
    case WATER:
      u8g2.setFontMode(1);
      u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(12+2+12+2, 12, 152);
      break;
  }
}

void display(uint8_t symbol) {
  draw_symbol(symbol, 1);
  u8g2.sendBuffer();
}

void blink(uint8_t symbol) {
  draw_symbol(symbol, 2);
  u8g2.sendBuffer();
}

void clear(uint8_t symbol) {
  draw_symbol(symbol, 0);
  u8g2.sendBuffer();
}

void display_reading(int reading) {
  sprintf(msg,"%3d", reading); 
    
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);
  //u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.setFont(u8g2_font_inb24_mr);
  u8g2.drawStr(u8g2.getDisplayWidth() - (u8g2.getMaxCharWidth()*3) - 14,u8g2.getDisplayHeight()-(u8g2.getMaxCharHeight()/4),msg);
  u8g2.setFont(u8g2_font_gb24st_t_1);
  u8g2.drawStr(115,13,"%");
  u8g2.sendBuffer();  
}

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
    blink(WIFI);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  display(WIFI);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Message arrived [home/water/level] 97,1 minute ago
  String message = String((char *) payload);
  if (message.indexOf(',') >= 0) {
    reading = message.substring(0, message.indexOf(',')).toInt();
  }
}

void setup_mqtt() {
  randomSeed(micros());
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect_() {
  // Loop until we're reconnected
  while (!client.connected()) {

    unsigned long currentMillis = millis();
    
    if(WiFi.status() != WL_CONNECTED) {
      blink(WIFI);
      clear(MQTT);
    } else {
      display(WIFI);
    }

    // Wait 5 seconds before retrying
    if(currentMillis - previousMillis >= 5000) {
      Serial.print("Attempting MQTT connection...");
      previousMillis = currentMillis;
      
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        client.subscribe("home/water/level");
        display(MQTT);
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
    }

    if(!client.connected()) {
      blink(MQTT);
    }
    delay(500);
  }
}


void setup(void) {
  Serial.begin(115200);
  u8g2.begin();
  setup_wifi();
  setup_mqtt();
}

void reconnect() {
  unsigned long currentMillis = millis();

  if((WiFi.status() == WL_CONNECTED) && (currentMillis - previousMillis >= 5000)) {
       
    Serial.print("Attempting MQTT connection...");
    
     // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("home/water/level");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      previousMillis = currentMillis;
    }
  }
}

void status() {
    if(WiFi.status() != WL_CONNECTED) {
      blink(WIFI);      
    } else {
      display(WIFI);
    }
  
    if (!client.connected()) {
      blink(MQTT);   
    } else {
      display(MQTT);
    }

    if(reading > -99 ) {
      display_reading(reading);
    }
}

void loop(void) {
    status();  
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    delay(500);
}

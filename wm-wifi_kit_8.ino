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
const char* ssid = "SAYANI_WIFI";//"VIVA-Router-ADV-LTE";
const char* password = "00011101";//"VIVA770319";

// MQTT configuration
const char* mqtt_server = "192.168.8.11";

// Min and max distance reading from the sensor
int min_distance = 15;
int max_distance = 78;

// Warning levels
const int warn_level = 20;
const int critical_level = 10;

// OLED
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);
//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

WiFiClient espClient;
PubSubClient client(espClient);
Ticker invertTicker;

unsigned long mqtt_last_conn_millis = 0;
unsigned long mqtt_last_message_millis = 0;
String water_level_percentage = "--";
char water_level_str[5];
char last_reading_str[10];
uint8_t foreground_color = 1;
uint8_t background_color = 0;

// Display symbols
void draw_symbol(uint8_t symbol, uint8_t color) {
  switch(symbol) {
    case WIFI:
      //u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(1, 12, 248);
      break;
    case MQTT:
      //u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(1, 12+2+12, 206);
      break;
    case WATER:
      u8g2.setDrawColor(color);
      u8g2.setFont(u8g2_font_open_iconic_all_1x_t);
      u8g2.drawGlyph(12+2+12+2, 12, 152);
      break;
  }
}

void display(uint8_t symbol) {
  draw_symbol(symbol, 1);
}

void blink(uint8_t symbol) {
  draw_symbol(symbol, 2);
  u8g2.sendBuffer(); //immediately
}

void clear0(uint8_t symbol) {
  draw_symbol(symbol, 0);
}

// Display water level
void display_reading() {
  unsigned long current_millis = millis();
  int secs = (current_millis - mqtt_last_message_millis) / 1000;
  int mins = max(60, secs) / 60;
  
  if(mins > 30 ) {
    water_level_percentage = "--";
  } 
  
  water_level_percentage.toCharArray(water_level_str, 5);

  //u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_logisoso24_tf);
  //u8g2.setFont(u8g2_font_ncenB14_tf);
  u8g2.drawStr(18,26,water_level_str);
}

// Display when a sensor reading was received
void display_ago() {
  if (mqtt_last_message_millis == 0) {
    return;
  }
  
  unsigned long current_millis = millis();
  int secs = (current_millis - mqtt_last_message_millis) / 1000;
  int mins = max(60, secs) / 60;

  if(mins >= 1440) {
    sprintf(last_reading_str,"%d %s", (mins / 1440), "day"); 
  } else if(mins >= 60) {
    sprintf(last_reading_str,"%d %s", (mins / 60), "hr"); 
  } else {
    sprintf(last_reading_str,"%d %s", mins, "min"); 
  }

  //u8g2.setDrawColor(2);
  u8g2.setFont(u8g2_font_mercutio_basic_nbp_tf);
  u8g2.drawStr(94,12,last_reading_str);
  u8g2.drawStr(94,26,"ago");
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
    //blink(WIFI);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //display(WIFI);
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

  // alarm
  alarm();
}

void alarm() {
  int level = water_level_percentage.toInt();
  Serial.println(level);
  if(level > 0) {
    if(level <= critical_level) {
       Serial.println("Critical level");
       tone(D3, 1000);
       delay(1000);
       noTone(D3);  
    } else if(level <= warn_level) {
       Serial.println("Warning level");
       tone(D3, 1000);
       delay(500);
       noTone(D3); 
    }
  }
}

// Initialize MQTT
void setup_mqtt() {
  randomSeed(micros());
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// Application setup
void setup(void) {
  Serial.begin(115200);
  u8g2.begin();
  setup_wifi();
  setup_mqtt();

  invertTicker.attach(900, invert); // invert color every X seconds
}

// Show emoji - not used now
void mood() {
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_emoticons21_tr);
  u8g2.drawGlyph(21+3, 21, 32);
}

// (Re)connect to MQTT
void reconnect() {
  unsigned long current_millis = millis();

  if((WiFi.status() == WL_CONNECTED) && (current_millis - mqtt_last_conn_millis >= 5000)) {
       
    Serial.print("Attempting MQTT connection...");
    
     // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) { // This blocks the thread
      Serial.println("connected");
      client.subscribe("home/monitor/water/level");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      mqtt_last_conn_millis = current_millis;
    }
  }
}

void invert() {
  foreground_color = (foreground_color == 1 ? 0 : 1);
  background_color = (background_color == 1 ? 0 : 1);
}

// Display symbols, level and ago
void status() {
    u8g2.clearBuffer();
    u8g2.setFontMode(1);
 
    u8g2.setDrawColor(background_color);
    u8g2.drawBox(0, 0, u8g2.getDisplayWidth(), u8g2.getDisplayHeight());

    u8g2.setDrawColor(foreground_color);
    if(WiFi.status() == WL_CONNECTED) {
      display(WIFI);
    }
  
    if (client.connected()) {
      display(MQTT);
    }

    display_reading();
    display_ago();

    u8g2.sendBuffer(); 
}

// Main loop
void loop(void) {
    status();  
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    delay(500);
}

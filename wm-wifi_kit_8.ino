#include <ESP8266WiFi.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const char* ssid = "SAYANI_IOT";
const char* password = "00001111";

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

bool wifi_connected = true;

void drawLogo(void) {
  
}

void drawSymbols(void) {
  u8g2.setFont(u8g2_font_open_iconic_all_2x_t);
  u8g2.drawGlyph(0, 24, 248);
  u8g2.drawGlyph(20, 24, 206);
  u8g2.drawGlyph(40, 24, 152);
}

void drawWaterLevel(void) {
  u8g2.setFont(u8g2_font_logisoso28_tr);
  u8g2.drawStr(54,30,"100%");
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
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void setup(void) {
  Serial.begin(115200);
  u8g2.begin();
  //setup_wifi();
}


void loop(void) {
    u8g2.clearBuffer();
    drawSymbols();
    drawWaterLevel();
    u8g2.sendBuffer();
    delay(1000);
}

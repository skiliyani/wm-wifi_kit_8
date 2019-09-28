#include <ESP8266WiFi.h>

const char* ssid = "SAYANI_IOT";
const char* password = "00001111";

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

  setup_wifi();
}


void loop(void) {
 
}

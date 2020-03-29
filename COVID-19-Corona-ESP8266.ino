/**

   Code example; for more informatio see:
   https://oneguyoneblog.com/


 **/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "SSD1306Brzo.h"
#include <ArduinoJson.h>

SSD1306Brzo display(0x3c, 5, 4);   // Initialize OLED display

const char* ssid = "yourssid";
const char* password = "yourpassword";

const char* host = "services1.arcgis.com";
String request = "/0MSEUqKaxRlEPj5g/arcgis/rest/services/Coronavirus_2019_nCoV_Cases/FeatureServer/1/query?where=Country_Region%20like%20'%25INDONESIA%25'&outFields=Last_Update,Confirmed,Deaths,Recovered&returnGeometry=false&outSR=4326&f=json";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "70580e780c9d727550619d3e4efdb21d64d1e91e"; //SHA1 finger print

void setup() {

  Serial.begin(115200);
  Serial.print("Connecting");

  delay(1000);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue
  delay(1000);
  WiFi.mode(WIFI_STA);        //Station mode
  WiFi.begin(ssid, password);     //Connect to WiFi

  display.init();   // Initialise the display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Connecting");
  display.display();    // Write the buffer to the display

  while (WiFi.status() != WL_CONNECTED) {  // Wait for connection
    delay(500);
    Serial.print(".");
  }

  Serial.println("");  //If connection successful show IP address of ESP8266 in serial monitor
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  Serial.println(host);
  Serial.printf("Using fingerprint '%s'\n", fingerprint);

  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);

  Serial.println("HTTPS Connecting");

  int r = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
    delay(100);
    Serial.print(".");
    r++;
  }
  
  if (r == 30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected");
  }

  Serial.print("Requesting: ");
  Serial.println(host + request);

  httpsClient.print(String("GET ") + request + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  Serial.println("Request sent");

  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  Serial.println("Payload received:");

  String payload;
  while (httpsClient.available()) {
    payload = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(payload); //Print response
  }

  Serial.println("Closing connection");

  char charBuf[500];
  payload.toCharArray(charBuf, 500);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 3 * JSON_OBJECT_SIZE(6) + 2 * JSON_OBJECT_SIZE(7) + 690;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, payload);

  JsonArray fields = doc["fields"];

  JsonObject features_0_attributes = doc["features"][0]["attributes"];

  String Confirmed = features_0_attributes["Confirmed"];
  String Deaths = features_0_attributes["Deaths"];
  String Recovered = features_0_attributes["Recovered"];
  long Last_Update = features_0_attributes["Last_Update"]; // not used yet

  display.clear();      // Clear OLED display
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  display.drawString(0, 0, "Conf: " + Confirmed);
  display.drawString(0, 24, "Dead: " + Deaths);
  display.drawString(0, 48, "Recv: " + Recovered);

  display.display();    // Write the buffer to the display

  delay(60000);
}

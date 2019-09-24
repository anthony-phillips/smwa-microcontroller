#include <ESP8266WiFi.h>

#define LED     2
#define LED_ON  LOW
#define LED_OFF HIGH
#define CT_PIN  A0

const char* WIFI_SSID     = "";
const char* WIFI_PASS     = "";
const byte  SERVER_ADDR[] = {10, 0, 1, 26};
const int   SERVER_PORT   = 5000;
const int   POLL_PERIOD   = 4000;

int  reading = 0;

void setup() {
  // Open serial port
  Serial.begin(115200);
  delay(100);
  Serial.println();

  // Enable LED pin
  pinMode(LED, OUTPUT);
  pinMode(CT_PIN, INPUT); // analog pin

  // TODO: Mount filesystem

  // TODO: Read configuration
 
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
}

void blinkLED(int duration=500) {    
  digitalWrite(LED, LED_ON);
  delay(duration);
  digitalWrite(LED, LED_OFF);
}
 
void loop() {
  delay(POLL_PERIOD);

  // Read analog pin
  reading = analogRead(CT_PIN);
 
  // Connect to the server using WiFi
  Serial.println("Connecting to server");
  WiFiClient client;
  if (!client.connect(SERVER_ADDR, SERVER_PORT)) {
    Serial.println("Connection failed");
    return;
  }
  
  // Create request URL
  String url = "/upload?reading=" + String(reading);
  
  // Send request to server
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Connection: close\r\n\r\n");
  blinkLED(250);
  
  Serial.println("Closing connection");
}

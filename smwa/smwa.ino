#include <ESP8266WiFi.h>

#define LED     2
#define LED_ON  LOW
#define LED_OFF HIGH
#define CT_PIN  A0

const char* WIFI_SSID     = "";
const char* WIFI_PASS     = "";
const int   MAX_CONN_ATT  = 1;

const byte  SERVER_ADDR[] = {10, 0, 1, 26};
const int   SERVER_PORT   = 5000;

const uint32_t  POLL_PERIOD = 4000;
const uint16_t  N       = 750;
const uint32_t  R_MAX   = 1024;
const double    V_MAIN  = 0.485;
const double    O_CT    = 21.0;
const uint16_t  N_CT    = 1800;
const double    V_DC    = 0.485;
const double    R_SCALE = 2;

double VCT_RIN[R_MAX];
double VCT_RIN_2[R_MAX];

void setup() {
  // Open serial port
  Serial.begin(115200);
  delay(100);
  Serial.println();

  // Generate the lookup tables
  int     dcOffset = R_MAX * V_DC;
  double  rInRatio = R_SCALE / R_MAX;
  
  Serial.print("DC Offset: ");
  Serial.println(dcOffset);
  Serial.print("Scale: ");
  Serial.println(rInRatio, 5);
  Serial.print("Negative Double: ");
  Serial.println(-3.14);
  
  for (int i=0; i < R_MAX; i++) {
    double vCT = (i - dcOffset) * rInRatio;
    VCT_RIN[i]   = vCT;
    VCT_RIN_2[i] = vCT * vCT;

    if (i%10 == 0){
      Serial.println(vCT, 5);
    } else {
      Serial.print(vCT, 5);
      Serial.print("   ");
    }
  }
  
  // Enable LED pin
  pinMode(LED, OUTPUT);
  pinMode(CT_PIN, INPUT); // analog pin

  // TODO: Mount filesystem

  // TODO: Read configuration
 
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int connAttempts = 0;
  while (connAttempts++ <= MAX_CONN_ATT &&
         WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  digitalWrite(LED, LED_OFF);
}

void blinkLED(int duration=500) {    
  digitalWrite(LED, LED_ON);
  delay(duration);
  digitalWrite(LED, LED_OFF);
}

double getWMain() {
  static double wVRatio = N_CT * V_MAIN / O_CT;
  double vrms = 0;
  double sigmaNSq = 0;

  digitalWrite(LED, LED_ON);
  // Get values
  for (int i=0; i < N; i++) {
    sigmaNSq += VCT_RIN_2[analogRead(CT_PIN)];
    delayMicroseconds(5); // Need to actually calculate this
  }

  vrms = sqrt(sigmaNSq / N);

  digitalWrite(LED, LED_OFF);
  return vrms * wVRatio;
}

void loop() {
  delay(POLL_PERIOD);

  // Read analog pin
  double reading = analogRead(CT_PIN);
  Serial.print("Analog reading: ");
  Serial.println(String(reading));
 
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

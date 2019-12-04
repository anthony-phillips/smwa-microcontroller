#include <ESP8266WiFi.h>

#define LED     2
#define LED_ON  LOW
#define LED_OFF HIGH
#define CT_PIN  A0

const char* WIFI_SSID     = "SMWA";
const char* WIFI_PASS     = "case.fox.pickle";
const int   MAX_CONN_ATT  = 1000;

const byte  SERVER_ADDR[] = {192, 168, 137, 1};
const int   SERVER_PORT   = 80;

const uint32_t  POLL_PERIOD = 1000; // Time between polls
const uint32_t  N       = 1000;     // Number of samples
const uint32_t  R_MAX   = 1024;     // ADC resolution
const double    V_DC    = 0.5;      // Input DC offset
const double    R_SCALE = 1;        // Input scale (Input/R_SCALE)
const double    V_MAIN  = 120.0;    // Main RMS voltage
const double    O_CT    = 21.0;     // CT burden resistance
const uint32_t  N_CT    = 1800;     // CT ratio

double VCT_RIN_2[R_MAX];  // Vct(Rin)^2

void setup() {
  // Open serial port
  Serial.begin(115200);
  delay(100);
  Serial.println();

  // Generate lookup tables
  int     dcOffset = R_MAX * V_DC;
  double  rInRatio = R_SCALE / R_MAX;
    
  for (int i=0; i < R_MAX; i++) {
    double vCT = (i - dcOffset) * rInRatio;
    VCT_RIN_2[i] = vCT * vCT;

    if (i%10 == 0){
      Serial.println(vCT, 5);
    } else {
      Serial.print(vCT, 5);
      Serial.print("   ");
    }
  }
  
  // Configure pins
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
    delayMicroseconds(2); // Need to actually calculate this
  }

  vrms = sqrt(sigmaNSq / N);

  Serial.print("Vrms: ");
  Serial.println(vrms, 5);
  digitalWrite(LED, LED_OFF);
  
  return vrms * wVRatio;
}

void loop() {
  delay(POLL_PERIOD);

  // Read analog pin
  double reading = getWMain();
  Serial.print("Wmain: ");
  Serial.println(String(reading));
 
  // Connect to the server using WiFi
  WiFiClient client;
  if (!client.connect(SERVER_ADDR, SERVER_PORT)) {
    Serial.println("Server connection failed");
    return;
  }
  
  // Create request URL
  String url = "/upload?reading=" + String(reading);
  
  // Send request to server
  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print("GET " + url + " HTTP/1.1\r\n" +
               "Connection: close\r\n\r\n");
}

#include <SoftwareSerial.h>
#include "config.h"

// Hardware Pin Definitions
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int GREEN_LED = 7;
const int RED_LED = 8;

// SoftwareSerial communication mapping for ESP8266 (RX, TX)
SoftwareSerial wifiSerial(2, 3); 

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  Serial.begin(9600);     // USB hardware serial debugging
  wifiSerial.begin(9600); // Communication port for ESP8266 module
  
  initializeWifi();
}

void loop() {
  long duration, distance;
  
  // Trigger the ultrasonic sensor pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure the travel duration of the echo sound wave
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.034) / 2; // Convert duration to distance in cm
  
  Serial.print("Current Fluid Distance Gap: ");
  Serial.print(distance);
  Serial.println(" cm");

  // State Management Logic based on Fluid Level
  if (distance >= DIST_CRITICAL || distance <= 0) {
    // Critical state: Fluid level low or reading error
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH); 
    sendCloudAlert(distance, "CRITICAL_LOW");
  } else {
    // Normal operation state
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    sendCloudAlert(distance, "NORMAL");
  }
  
  delay(5000); // Sample telemetry readings every 5 seconds
}

void initializeWifi() {
  Serial.println("Configuring network connection...");
  wifiSerial.println("AT+RST");
  delay(2000);
  
  String connectCmd = "AT+CWJAP=\"";
  connectCmd += WIFI_SSID;
  connectCmd += "\",\"";
  connectCmd += WIFI_PASSWORD;
  connectCmd += "\"";
  
  wifiSerial.println(connectCmd);
  delay(5000); // Allow time to negotiate IP address acquisition
}

void sendCloudAlert(int currentLevel, String statusText) {
  // Establish basic TCP routing via AT Commands
  String tcpStart = "AT+CIPSTART=\"TCP\",\"";
  tcpStart += IOT_API_HOST;
  tcpStart += "\",80";
  
  wifiSerial.println(tcpStart);
  delay(1000);
  
  // Construct HTTP payload string
  String httpPayload = "GET /update?api_key=";
  httpPayload += API_KEY;
  httpPayload += "&field1=";
  httpPayload += String(currentLevel);
  httpPayload += " HTTP/1.1\r\nHost: ";
  httpPayload += IOT_API_HOST;
  httpPayload += "\r\n\r\n";
  
  String sendCmd = "AT+CIPSEND=";
  sendCmd += httpPayload.length();
  
  wifiSerial.println(sendCmd);
  delay(500);
  wifiSerial.print(httpPayload);
}

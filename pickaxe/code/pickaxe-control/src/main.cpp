#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define BUTTON1_PIN D1  // Pin for button 1
#define BUTTON2_PIN D2  // Pin for button 2

// Variables to store button states
bool lastButton1State = HIGH; // Assuming pull-up configuration (HIGH means not pressed)
bool lastButton2State = HIGH;
bool currentButton1State;
bool currentButton2State;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay = 50; // 50ms debounce delay


const char* ssid = "cuidadoconlamina";
const char* password = "12345678";

// Server IP address (change this to your server's IP)
const char* serverIP = "192.168.4.1"; // Replace with your server IP
int serverPort = 80; // HTTP port (default is 80)

// Function to send HTTP request to server
void sendHttpRequest(int button) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    // Create the URL for the GET request
    String url = "http://" + String(serverIP) + "/button?pressed=" + String(button);

    // Send HTTP GET request
    http.begin(client, url);
    http.GET();  // Send the GET request
    http.end();  // Close connection
  }
}

void setup() {  
  // Turn off the LED initially
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);

  // Initialize the buttons as inputs with pull-ups
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);           // Set NodeMCU ESP12E to station mode
  WiFi.begin(ssid, password);    // Connect to the specified Wi-Fi network

  // Flash the LED while waiting for Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {  // Wait until the ESP connects to Wi-Fi
    digitalWrite(16, HIGH);  // Turn the LED on
    delay(300);              // Wait for half a second
    digitalWrite(16, LOW);   // Turn the LED off
    delay(300);              // Wait for another half a second
  }

  for (int i = 0; i < 10; i++) {
    digitalWrite(16, i % 2);
    delay(50);
  }
  digitalWrite(16, HIGH);
}

int buttonSta=LOW;  

void loop() {
  // Read the current state of the buttons
  int reading1 = digitalRead(BUTTON1_PIN);
  int reading2 = digitalRead(BUTTON2_PIN);

   // Check if button 1 state has changed and debounce
  if (reading1 != lastButton1State) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (reading1 != currentButton1State) {
      currentButton1State = reading1;
      if (currentButton1State == LOW) {  // Button 1 pressed
        sendHttpRequest(1);  // Send HTTP request for Button 2
      }
    }
  }

  // Check if button 2 state has changed and debounce
  if (reading2 != lastButton2State) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != currentButton2State) {
      currentButton2State = reading2;
      if (currentButton2State == LOW) {  // Button 2 pressed
        sendHttpRequest(2);  // Send HTTP request for Button 2
      }
    }
  }

  // Update the last button states
  lastButton1State = reading1;
  lastButton2State = reading2;
  delay(5);
}

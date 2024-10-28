#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "cuidadoconlamina";
const char* password = "12345678";

#define RELAY_PIN 14

int interval = 45;
unsigned long previousMillis = 0;  // Stores the last time the LED state was updated

bool relayValue = false;

AsyncWebServer server(80);
String btDevicesList = "[]";
String fileList = "[]";  // Store the list of files here


// Function to handle button press requests
void handleButtonPress(AsyncWebServerRequest *request) {
  if (request->hasParam("pressed")) {
    String button = request->getParam("pressed")->value();

    if (button == "2") {
      relayValue = !relayValue;
    }
    
    // No need to send any acknowledgment to the client
    request->send(200);  // Optionally send a 200 OK response
  } else {
    // Handle the case where the "pressed" parameter is missing
    request->send(400, "text/plain", "Missing 'pressed' parameter");
  }
}

void setup() {

  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);

  randomSeed(analogRead(0));

  // Initialize Wi-Fi as an access point
  WiFi.softAP(ssid, password);

  // Define HTTP GET endpoint for "/button"
  server.on("/button", handleButtonPress);

  // Start the server
  server.begin();
}

void relayLightingLoop() {
  unsigned long currentMillis = millis();  // Get the current time in milliseconds

  // Check if 50ms have passed since the last update
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Save the last update time

    // Generate a random value (0 or 1) and set it to the LED pin
    int randomValue = random(0, 2);  // Random value between 0 and 1
    digitalWrite(RELAY_PIN, randomValue == 1 ? HIGH : LOW);
  }
}

void loop() {
  if (relayValue) {
    relayLightingLoop();
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}

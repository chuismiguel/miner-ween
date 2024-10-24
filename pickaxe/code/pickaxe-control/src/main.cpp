#include <Arduino.h>

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

void setup() {
  Serial.begin(115200); // Initialize serial for debugging output
  pinMode(BUTTON1_PIN, INPUT_PULLUP); // Configure button pins with internal pull-up resistors
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
}

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
      if (currentButton1State == LOW) { // LOW means button is pressed
        Serial.println("Button 1 pressed!");
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
      if (currentButton2State == LOW) {
        Serial.println("Button 2 pressed!");
      }
    }
  }

  // Update the last button states
  lastButton1State = reading1;
  lastButton2State = reading2;
}

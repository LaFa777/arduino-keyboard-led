#include <Arduino.h>

const int ledPin = 11;
bool currentKeyboardLayout = true;
int serialBaudRate = 9600;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(serialBaudRate);
}

void loop() {
  if (Serial.available())
  {
    int currentKeyboardLayout = Serial.read();
    if(currentKeyboardLayout)
      analogWrite(ledPin, 92);
    else
      analogWrite(ledPin, 0);
  }
}
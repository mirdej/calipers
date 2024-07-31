// Calipers - simple
// Basic usage of the digital caliper Library
// © 2024 Michael Egger AT anyma.ch
// MIT License

#include "Calipers.h"

// define clock and data pins
const int PIN_CLOCK = 1;
const int PIN_DATA = 2;

// Instance of the calipers class
Calipers caliper;


void setup() {
  caliper.begin(PIN_DATA, PIN_CLOCK);
}

void loop() {
  if (caliper.available()) // call this function repeatedly - it does the actual processing of data
  {
    Serial.println(caliper.get_mm(), 2);
  }
  delay(4);
}

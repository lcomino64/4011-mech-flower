#include <Adafruit_CircuitPlayground.h>

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
}

// Set the current blossom position + led colour spectrum position
void set_bloom_level(int level) {
  for (int i = 0; i < 11; i++) {
    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(level));
  }
  delay(10);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 115; i < 260; i++) {
    set_bloom_level(i);
  }
  CircuitPlayground.clearPixels();
  for (int i = 260; i > 115; i--) {
    set_bloom_level(i);
  }
  CircuitPlayground.clearPixels();
}

#include <Adafruit_CircuitPlayground.h>

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
  Serial1.begin(115200);
}

// Set the current blossom position + led colour spectrum position
void set_bloom_level(int level) {
  for (int i = 0; i < 11; i++) {
    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(level));
  }
  delay(10);
}

void loop() {
  if (Serial1.available()) {
    String input = Serial1.readStringUntil('\n');  // Read the line from the serial port
    Serial1.println(input);
    input.trim();

    int value = input.toInt();  // Convert the input string to an integer

    // Check if the conversion was successful
    if (input.length() > 0) {
      // Print the converted integer
      set_bloom_level(map(value, 0, 100, 260, 160));
    }
  }
}
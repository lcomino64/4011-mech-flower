#include "Adafruit_Crickit.h"
#include "ArduinoJson.h"
#include "HttpClient.h"
#include "seesaw_servo.h"

Adafruit_Crickit crickit;
seesaw_Servo stem_servo(&crickit); // create servo object to control a servo

HttpClient http;

http_request_t request;
http_response_t response;
http_header_t device_token;

http_header_t headers[] = {
    {"device-token", "2f6dd09c-7e19-44bc-995b-39f79ea0b21a"},
    {NULL, NULL} // NOTE: Always terminate headers with NULL
};

void setup() {
    // Serial.begin(115200);
    Serial1.begin(115200);
    crickit.begin();

    stem_servo.attach(CRICKIT_SERVO1); // attaches the servo to CRICKIT_SERVO1 pin

    set_petal_level(0); // default pos, fully open

    WiFi.on();
    WiFi.connect();
    waitUntil(WiFi.ready);

    // Set up the request to point to the URL
    request.hostname = "api.tago.io";
    request.port = 80;
    request.path = "/data?variable=airquality&query=last_item";
}

int val_map(int value, int fromLow, int fromHigh, int toLow, int toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

void set_petal_level(int level) {
    int level_scaled = val_map(level, 0, 100, 90, 180);
    stem_servo.write(level_scaled);
}

void loop() {
    // Send the GET request
    http.get(request, response, headers);

    // Check the response code
    if (response.status == 200) {
        // The request was successful
        String body = response.body;

        StaticJsonDocument<512> doc;

        // Deserialize the JSON string
        DeserializationError error = deserializeJson(doc, body);

        // Check for parsing errors
        if (!error) {
            // Extract the value header
            String value = doc["result"][0]["value"];
            Serial1.printf(value);
            Serial1.printf("\n");
            set_petal_level(value.toInt());
        }
    }

    // Wait for 5 seconds before sending the next request
    delay(100);
}

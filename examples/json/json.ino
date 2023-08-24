#include <ArduinoJson.h>
#include <Wire.h>
#ifndef I2C_SDA
#define I2C_SDA SDA
#endif
#ifndef I2C_SCL
#define I2C_SCL SCL
#endif

#include "SDx.h"
SDx sdx;

#include "IWA.h"
IWA input;

File datafile;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.printf("\nSD JSON Test\n");

    if (!sdx.begin()) {
        Serial.printf("SD initialization failed: %s\n", sdx._error.c_str());
        delay(10000);
        exit(0);
    } else {
        Serial.printf("SD initialized successfully, using file: %s\n", sdx._filename);
    }

    Wire.setPins(I2C_SDA, I2C_SCL);
    Wire.begin();

    if (input.begin()) {
        Serial.println("IWA initialized successfully.");
    } else {
        Serial.println("Failed to initialize IWA!");
        exit(0);
    }
    sdx.listDir("/", 0);
}

void loop() {
    StaticJsonDocument<256> doc;
    JsonObject root = doc.to<JsonObject>();
    JsonArray dataArray = root.createNestedArray("Time");
    JsonObject dataSet = dataArray.createNestedObject();
    dataSet["name"] = "Elapsed";
    dataSet["value"] = millis();
    dataSet["unit"] = "ms";
    if (input.getJSON(root)) {
        sdx.setJSON(root);
    } else {
        Serial.println("Failed to get IWA data.");
    }

    sdx.loopSD();

    delay(1000);
}

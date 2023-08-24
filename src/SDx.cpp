/*!
 * @file SDx.cpp
 * @brief Set data to SD Card, and communicate with Python program
 * @n ...
 * @copyright   MIT License
 * @author [Bjarke Gotfredsen](bjarke@gotfredsen.com)
 * @version  V1.0
 * @date  2023
 * @https://github.com/domino4com/SD
 */
#include "SDx.h"
#define DDD Serial.printf("Line# %d\n", __LINE__);

SPIClass hspi(HSPI);

SDx::SDx() {
}

bool SDx::begin(uint8_t CS, uint8_t SCK, uint8_t MISO, uint8_t MOSI) {
    hspi.begin(SCK, MISO, MOSI, CS);
    if (!SD.begin(CS, hspi)) {
        _error = "Card Mount Failed";
        return false;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        _error = "No SD card attached";
        return false;
    }

    if (cardType == CARD_MMC) {
        _cardType = "MMC";
    } else if (cardType == CARD_SD) {
        _cardType = "SDSC";
    } else if (cardType == CARD_SDHC) {
        _cardType = "SDHC";
    } else {
        _cardType = "UNKNOWN";
    }

    _cardSize = SD.cardSize() / (1024 * 1024);
    _totalSpace = SD.totalBytes() / (1024 * 1024);
    _usedSpace = SD.usedBytes() / (1024 * 1024);

    int i = 0;
    do {
        sprintf(_filename, "/data%04d.json", i++);
        if (i > 9999) {
            _error = "No free filenames";
            return false;
        }
    } while (SD.exists(_filename));

    return true;
}

bool SDx::setJSON(JsonObject &doc) {
    File file = SD.open(_filename, FILE_APPEND);
    serializeJson(doc, file);
    file.println(",");
    return true;
}

void SDx::listDir(const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);
    File root = SD.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void SDx::deleteAll() {
    char buf[20];
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            Serial.printf("Deleting file: %s\n", file.name());
            sprintf(buf, "/%s", file.name());
            SD.remove(buf);
        }
        file = root.openNextFile();
    }
    root.close();
}

void SDx::loopSD() {
    if (Serial.available()) {
        Serial.println();
        String command = Serial.readStringUntil('\n');
        if (command.startsWith("DELETEALL")) {
            deleteAll();
        }
        if (command.startsWith("LISTDIR")) {
            listDir("/", 0);
        }
        if (command.startsWith("GET_FILE")) {
            String fileName = "/" + command.substring(9);
            File myFile = SD.open(fileName);

            if (myFile) {
                Serial.println("START_OF_FILE");
                while (myFile.available()) {
                    Serial.write(myFile.read());
                }
                myFile.close();
                Serial.flush();
                Serial.println("END_OF_FILE");
            } else {
                Serial.println("Error opening file!");
            }
        }
    }
}
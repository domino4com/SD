#ifndef SDx_H
#define SDx_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>
#include <Wire.h>

class SDx {
   public:
    SDx();
    bool begin(uint8_t CS = 5, uint8_t SCK = 14, uint8_t MISO = 12, uint8_t MOSI = 13);
    bool setJSON(JsonObject &doc);
    void listDir(const char * dirname, uint8_t levels);
    void deleteAll();
    void loopSD();
    uint64_t _cardSize;
    uint64_t _totalSpace;
    uint64_t _usedSpace;
    String _cardType;
    String _error;
    char _filename[20];

   private:


};
#endif  // SD_H
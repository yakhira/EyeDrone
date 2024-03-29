#ifndef UTILS_H_
#define UTILS_H_

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "ESP32httpUpdate.h"

class ESPUtils {
    public:
        void saveFile(String filename, JSONVar data);
        void saveFile(String filename, String data);
        void readFile(String filename, JSONVar &data);
        void readFile(String filename, String &data);
        void listDir(const char * dirname, uint8_t levels);
        void removeFile(String filename);

        bool getHTTPData(String url, String &result);
        bool downloadFile(String url, String filename);
        bool getHTTPJsonData(String url, JSONVar &result);
        bool sendHTTPJsonData(String url, JSONVar data);

        void removeString(String string, String from, String to, String &result);

        bool fsMounted = false;

        void mountFS();

        t_httpUpdate_return updateSketch(String url);

        WiFiClient client;
        WiFiClientSecure clientSecure;

        ESPUtils();
        ~ESPUtils();
};

#endif
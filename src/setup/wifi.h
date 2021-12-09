#ifndef WIFI_H_
#define WIFI_H_

#include "utils/utils.h"

class ESPWiFi: public ESPUtils {
    private:
        static const char *defaultWifiPassword;
        static String espChipName;
        static const String configFile;

        bool isWebServerRunning;

    public:
        ESPWiFi(String chipName);
        ~ESPWiFi();

        static JSONVar wifiConfig;
        static void handleMain();
        static void handleSave();
        static void handleUpdateSketch();
        static char *genUniqueHostname(String prefix, String suffix);

        void wifiConnect();
        void stateCheck();

        t_httpUpdate_return updateSketch(int sketch_version);
};

#endif
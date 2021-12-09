#include <Arduino.h>
#include <Arduino_JSON.h>

#include "wifi.h"

#if defined(ESP32)
#include <WebServer.h>
WebServer server(8080);
#else
ESP8266WebServer server(8080);
#endif

JSONVar ESPWiFi::wifiConfig;
String ESPWiFi::espChipName;
const char *ESPWiFi::defaultWifiPassword = "ESPp@$$w0rd!";
const String ESPWiFi::configFile = "/wifi_config.json";

ESPWiFi::ESPWiFi(String chipName){
    isWebServerRunning = false;
    espChipName = chipName;
}

ESPWiFi::~ESPWiFi(){

}

char *ESPWiFi::genUniqueHostname(String prefix, String macAddress){
	macAddress.replace(":", "");

	String s_hostname = prefix + "-" + macAddress;
    unsigned int len = s_hostname.length() + 1;
    char hostname[len];

	s_hostname.toCharArray(hostname, len);

	return hostname;
}

void ESPWiFi::wifiConnect(){
    IPAddress staticIP;
    IPAddress dns;
    IPAddress gateway;
    IPAddress subnet;

    char *hostname = genUniqueHostname("ESP-AP", WiFi.macAddress());

    readFile(configFile, wifiConfig);

    if (wifiConfig.hasOwnProperty("wifi_ssid") && wifiConfig.hasOwnProperty("wifi_password"))
    {
        WiFi.begin(wifiConfig["wifi_ssid"], wifiConfig["wifi_password"]);

        if (wifiConfig.hasOwnProperty("staticIP")){
            staticIP.fromString(wifiConfig["staticIP"]);
            dns.fromString(wifiConfig["dns"]);
            gateway.fromString(wifiConfig["gateway"]);
            subnet.fromString(wifiConfig["subnet"]);
            WiFi.config(staticIP, dns, gateway, subnet);
        }

        if (!wifiConfig.hasOwnProperty("hostname")) {
            wifiConfig["hostname"] = hostname;
        }
        else if (wifiConfig["hostname"].length() <= 0)
        {
            wifiConfig["hostname"] = hostname;
        }

        WiFi.mode(WIFI_STA);
        WiFi.hostname(JSON.stringify(wifiConfig["hostname"]));
        WiFi.setAutoReconnect(true);

        while (WiFi.status() != WL_CONNECTED){
            stateCheck();
            delay(500);
        }

        if (!wifiConfig.hasOwnProperty("staticIP")){
            wifiConfig["staticIP"] = WiFi.localIP().toString();
            wifiConfig["dns"] = WiFi.dnsIP().toString();
            wifiConfig["gateway"] = WiFi.gatewayIP().toString();
            wifiConfig["subnet"] = WiFi.subnetMask().toString();

            saveFile(configFile, wifiConfig);
        }
    }
    else {
        LittleFS.format();

        WiFi.softAP(hostname, defaultWifiPassword);

        server.on("/", HTTP_GET, ESPWiFi::handleMain);
        server.on("/wifi/save", HTTP_POST, ESPWiFi::handleSave);
        server.on("/update", HTTP_GET, ESPWiFi::handleUpdateSketch);
        server.begin();

        isWebServerRunning = true;
    }
}

void ESPWiFi::handleSave(){
    if (server.hasArg("wifi_ssid") && server.hasArg("wifi_password")){
        ESPUtils esputils;
        ESPWiFi::wifiConfig["wifi_ssid"] = server.arg("wifi_ssid");
        ESPWiFi::wifiConfig["wifi_password"] = server.arg("wifi_password");
        ESPWiFi::wifiConfig["ota_update_url"] = server.arg("ota_update_url");
        ESPWiFi::wifiConfig["sleep_state_url"] = server.arg("sleep_state_url");
        ESPWiFi::wifiConfig["hostname"] = server.arg("hostname");

        esputils.saveFile(ESPWiFi::configFile,  ESPWiFi::wifiConfig);

        if (server.arg("restart_device") == "on") {
            server.send(200, "text/html", "Accepted. Restarting device automatically in 3 seconds.");
            delay(3000);
        #if defined(ESP32) 
            ESP.restart();
        #else
            ESP.reset();
        #endif
        } else {
            server.send(200, "text/html", "Accepted. Please restart device.");
        }
    } else {
        server.send(400, "text/html", "Missing fields WIFI SSID or WIFI PASSWORD.");
    }
}

void ESPWiFi::handleMain(){
    ESPUtils esputils;
    String data = "<html>"
        "<title>Setup wifi</title>"
        "<body>"
        "    <form action=\"/wifi/save\" method=\"POST\">"
        "        <table>"
        "            <tr>"
        "                <td>WIFI SSID:</td><td><input type=\"text\" name=\"wifi_ssid\"></td>"
        "            </tr>"
        "            <tr>"
        "                <td>WIFI PASSWORD:</td><td><input type=\"password\" name=\"wifi_password\"></td>"
        "            </tr>"
        "            <tr>"
        "                <td>HOSTNAME:</td><td><input type=\"text\" name=\"hostname\"></td>"
        "            </tr>"
        "            <tr>"
        "                <td>OTA UPDATE URL (optional):</td><td><input type=\"text\" name=\"ota_update_url\"></td>"
        "            </tr>"
        "            <tr>"
        "                <td>SLEEP STATE URL (optional):</td><td><input type=\"text\" name=\"sleep_state_url\"></td>"
        "            </tr>"
        "            <tr>"
        "                <td></td><td style=\"text-align: right\"><input type=\"checkbox\" name=\"restart_device\">Restart device</td>"
        "            </tr>"
        "            <tr>"
        "                <td></td><td style=\"text-align: right\"><input type=\"submit\" value=\"save\"></td>"
        "            </tr>"
        "        </table>"
        "    </form><br>"
        "</body>"
        "</html>";

    server.send(200, "text/html", data);
}

void ESPWiFi::handleUpdateSketch(){
    ESPUtils esputils;
    server.send(500, "application/json", "{\"status\": \"UPDATING\"}");
    esputils.updateSketch(wifiConfig["ota_update_url"] + "?chip=" + espChipName + "&mac=" + WiFi.macAddress() + "&version=" + -1);
}

void ESPWiFi::stateCheck(){
    if (digitalRead(0) == LOW){
        mountFS();
        LittleFS.remove(configFile);
    }
    if (isWebServerRunning) {
        server.handleClient();
    }
}

t_httpUpdate_return ESPWiFi::updateSketch(int sketch_version){
    return ESPUtils::updateSketch(
        wifiConfig["ota_update_url"] + "?chip=" + espChipName + "&mac=" + WiFi.macAddress() + "&version=" + sketch_version
    );
}
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/adc.h"
#include "setup/wifi.h"
#include "car/car.h"

#define LEFT_BACK     15
#define LEFT_FORWARD  14
#define RIGHT_FORWARD 13
#define RIGHT_BACK    12
#define SPEED         2
#define LED           4

// -------- DEFAULT SKETCH PARAMETERS --------
const int SKETCH_VERSION = 23;

ESPWiFi espwifi("ESP32-D0WDQ5");
ESPCar espcar(
	LEFT_BACK,
	LEFT_FORWARD,
	RIGHT_BACK,
	RIGHT_FORWARD,
	SPEED,
	LED
);

void main_code()
{
	espcar.begin();
	Serial.println("URL: http://" + WiFi.localIP().toString());
}

void setup(){
	pinMode(LEFT_BACK, OUTPUT);
	pinMode(LEFT_FORWARD, OUTPUT);
	pinMode(RIGHT_BACK, OUTPUT);
	pinMode(RIGHT_FORWARD, OUTPUT);
	pinMode(LED, OUTPUT); 

	Serial.begin(115200);

	espwifi.wifiConnect();

	WiFi.setSleep(WIFI_PS_NONE);

	if (WiFi.getMode() == WIFI_STA) {
		espwifi.updateSketch(SKETCH_VERSION);
		main_code();
	}
}

void loop(){
	if (WiFi.getMode() == WIFI_STA) {
		if (WiFi.status() != WL_CONNECTED) {
			WiFi.reconnect();
		}
	}
}
#include "esp_camera.h"
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
const int SKETCH_VERSION = 16;

ESPWiFi espwifi("ESP32-D0WDQ5");
ESPCar espcar(
	LEFT_BACK,
	LEFT_FORWARD,
	RIGHT_BACK,
	RIGHT_FORWARD,
	SPEED,
	LED
);

unsigned long last_time = 0;
unsigned long http_errors_count = 0;

void main_code()
{
	espcar.begin();
	Serial.println("URL: http://" + WiFi.localIP().toString());
}

void checkSleepState(unsigned int interval){
	if (last_time == 0) {
		last_time = millis()/1000;
	}

	if ((last_time + interval) <= (millis()/1000)) {
		JSONVar sleepState;
		if (!espwifi.getHTTPJsonData(
			espwifi.dataUrl + "/esp/sleep?mac=" + WiFi.macAddress(),
			sleepState
		)) {
			http_errors_count +=1;
		} else {
			http_errors_count = 0;
		}

		if ((bool)sleepState["state"] || http_errors_count > 10){
			adc_power_off();

			digitalWrite(LEFT_BACK, LOW);
			digitalWrite(LEFT_FORWARD, LOW);
			digitalWrite(RIGHT_BACK, LOW);
			digitalWrite(RIGHT_FORWARD, LOW);
			digitalWrite(LED, LOW);

			gpio_deep_sleep_hold_en();

			gpio_hold_en((gpio_num_t) LEFT_BACK);
			gpio_hold_en((gpio_num_t) LEFT_FORWARD);
			gpio_hold_en((gpio_num_t) RIGHT_BACK);
			gpio_hold_en((gpio_num_t) RIGHT_FORWARD);
			gpio_hold_en((gpio_num_t) LED);

			esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
			esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
			esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

			esp_sleep_enable_timer_wakeup((int)sleepState["interval"]*1000000ULL);	
			esp_deep_sleep_start();
		}
		last_time = millis()/1000;
	}
}

void setup(){
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

	gpio_hold_dis((gpio_num_t)LEFT_BACK);
	gpio_hold_dis((gpio_num_t)LEFT_FORWARD);
	gpio_hold_dis((gpio_num_t)RIGHT_BACK);
	gpio_hold_dis((gpio_num_t)RIGHT_FORWARD);
	gpio_hold_dis((gpio_num_t)LED);

	pinMode(LEFT_BACK, OUTPUT);
	pinMode(LEFT_FORWARD, OUTPUT);
	pinMode(RIGHT_BACK, OUTPUT);
	pinMode(RIGHT_FORWARD, OUTPUT);
	pinMode(LED, OUTPUT);  

	Serial.begin(115200);

	espwifi.wifiConnect();

	WiFi.setSleep(WIFI_PS_NONE);

	if (WiFi.getMode() == WIFI_STA) {
		checkSleepState(0);
		espwifi.updateSketch(SKETCH_VERSION);
		main_code();
	}
}

void loop(){
	if (WiFi.getMode() == WIFI_STA) {
		if (WiFi.status() != WL_CONNECTED) {
			WiFi.reconnect();
		}
		checkSleepState(30);
	}

	espwifi.stateCheck();
}
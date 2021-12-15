#include "setup/wifi.h"
#include "esp_camera.h"

// -------- DEFAULT SKETCH PARAMETERS --------
const int SKETCH_VERSION = 4;

ESPWiFi espwifi("ESP32-D0WDQ5");

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

// GPIO Setting
extern int gpLb = 14;  // Left Wheel Back
extern int gpLf = 15; // Left Wheel Forward
extern int gpRb = 13; // Right Wheel Back
extern int gpRf = 12; // Right Wheel Forward
extern int gpLed = 2; // Light
extern String WiFiAddr ="";

void startCameraServer();

void main_code()
{
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = Y2_GPIO_NUM;
	config.pin_d1 = Y3_GPIO_NUM;
	config.pin_d2 = Y4_GPIO_NUM;
	config.pin_d3 = Y5_GPIO_NUM;
	config.pin_d4 = Y6_GPIO_NUM;
	config.pin_d5 = Y7_GPIO_NUM;
	config.pin_d6 = Y8_GPIO_NUM;
	config.pin_d7 = Y9_GPIO_NUM;
	config.pin_xclk = XCLK_GPIO_NUM;
	config.pin_pclk = PCLK_GPIO_NUM;
	config.pin_vsync = VSYNC_GPIO_NUM;
	config.pin_href = HREF_GPIO_NUM;
	config.pin_sscb_sda = SIOD_GPIO_NUM;
	config.pin_sscb_scl = SIOC_GPIO_NUM;
	config.pin_pwdn = PWDN_GPIO_NUM;
	config.pin_reset = RESET_GPIO_NUM;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;
	
	//init with high specs to pre-allocate larger buffers
	if (psramFound())
	{
		config.frame_size = FRAMESIZE_UXGA;
		config.jpeg_quality = 10;
		config.fb_count = 2;
	}
	else
	{
		config.frame_size = FRAMESIZE_SVGA;
		config.jpeg_quality = 12;
		config.fb_count = 1;
	}

	// play setting
	config.jpeg_quality = 10;
	config.fb_count = 3;

	// camera init
	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK)
	{
		return;
	}

	sensor_t *s = esp_camera_sensor_get();
	s->set_framesize(s, FRAMESIZE_QVGA);

 	startCameraServer();
	WiFiAddr = WiFi.localIP().toString();
}

void checkSleepState(unsigned int sleep){
	JSONVar sleepState;
	espwifi.getHTTPJsonData(espwifi.dataUrl + "/esp/sleep?mac=" + WiFi.macAddress(), sleepState);

	if ((bool)sleepState["state"]){
		digitalWrite(gpLed, LOW);

		gpio_hold_en((gpio_num_t) gpLb);
		gpio_hold_en((gpio_num_t) gpLf);
		gpio_hold_en((gpio_num_t) gpRb);
		gpio_hold_en((gpio_num_t) gpRf);
		gpio_hold_en((gpio_num_t) gpLed);
		gpio_deep_sleep_hold_en();
		esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
		esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
		esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

		esp_sleep_enable_timer_wakeup((int)sleepState["interval"]*1000000);	
		esp_deep_sleep_start();
	} else {
		delay(sleep*1000);
	}
}

void setup()
{
	Serial.begin(115200);

	gpio_hold_dis((gpio_num_t)gpLb);
	gpio_hold_dis((gpio_num_t)gpLf);
	gpio_hold_dis((gpio_num_t)gpRb);
	gpio_hold_dis((gpio_num_t)gpRf);
	gpio_hold_dis((gpio_num_t)gpLed);

	pinMode(gpLb, OUTPUT);
	pinMode(gpLf, OUTPUT);
	pinMode(gpRb, OUTPUT);
	pinMode(gpRf, OUTPUT);
	pinMode(gpLed, OUTPUT);

	digitalWrite(gpLb, LOW);
	digitalWrite(gpLf, LOW);
	digitalWrite(gpRb, LOW);
	digitalWrite(gpRf, LOW);
	digitalWrite(gpLed, LOW);

	espwifi.wifiConnect();

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
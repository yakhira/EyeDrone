#include <Arduino.h>
#include <Arduino_JSON.h>

#include "car.h"

ESPCar::ESPCar(){
    carLed = 0;
    carSpeed = 2;
    carLeftForward = 15;
    carLeftBack = 14;
    carRightBack = 13;
    carRightForward = 12;

    server = new AsyncWebServer(80);
}

ESPCar::ESPCar(int leftBack, int leftForward, int rightBack, int rightForward, int speed, int led){
    carLed = led;

    carSpeed = speed;
    carLeftBack = leftBack;
    carLeftForward = leftForward;
    carRightBack = rightBack;
    carRightForward = rightForward;

    server = new AsyncWebServer(80);
}

ESPCar::~ESPCar(){
}

void ESPCar::begin()
{
    int leftBack = carLeftBack;
    int leftForward = carLeftForward;
    int rightBack = carRightBack;
    int rightForward = carRightForward;

    int led = carLed;

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
	
	config.frame_size = FRAMESIZE_VGA;
	config.jpeg_quality = 10;
	config.fb_count = 1;

	esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
	{
		Serial.println("Can't init camera.");
	}

    // ledcSetup(7, 5000, 8);
    // ledcAttachPin(4, 7);

    ledcAttachPin(carSpeed, 8);
    ledcSetup(8, 2000, 8);      
    ledcWrite(8, 130);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/html/index.html", "text/html");
    });

    server->on(
        "/drive", HTTP_GET, [leftBack, leftForward, rightBack, rightForward](
            AsyncWebServerRequest *request){
                int nLf = LOW, nLb = LOW, nRf = LOW, nRb = LOW, speed = 255;
                if (request->hasArg("speed")) {
                    speed = request->arg("speed").toInt();

                    if (speed > 255) {
                        speed = 255;
                    }  else if (speed < 0) {
                        speed = 0;
                    }
                    ledcWrite(8, speed);
                }
                if (request->hasArg("left")) {
                    nLf = LOW, nLb = HIGH, nRf = HIGH, nRb = LOW;
                } else if (request->hasArg("right")) {
                    nLf = HIGH, nLb = LOW, nRf = LOW, nRb = HIGH;
                } else if (request->hasArg("go")) {
                    nLf = LOW, nLb = HIGH, nRf = LOW, nRb = HIGH;
                } else if (request->hasArg("back")) {
                    nLf = HIGH, nLb = LOW, nRf = HIGH, nRb = LOW;
                } else if (request->hasArg("stop")) {
                    nLf = LOW, nLb = LOW, nRf = LOW, nRb = LOW;
                }

                digitalWrite(leftBack, nLb);
                digitalWrite(leftForward, nLf);
                digitalWrite(rightBack, nRb);
                digitalWrite(rightForward, nRf);

                request->send(200, "text/html", "OK");
            }
        );
    
    server->on("/led", HTTP_GET, [led](AsyncWebServerRequest *request){
        if (request->hasArg("on")){
            digitalWrite(led, HIGH);
        } else {
            digitalWrite(led, LOW);
        } 
        request->send(200, "text/html", "OK");
    });

    server->on("/stream", HTTP_GET, [] (AsyncWebServerRequest *request) {
        AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
        if(!response){
            request->send(501);
            return;
        }
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    server->begin();
}

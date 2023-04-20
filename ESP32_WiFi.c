#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include <driver/adc.h>

const char ssid = "pr - dr3";
const char password = "welcome1";

void init_WiFi(){
    int ledState = 0;
    gpio_reset_pin(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 0);

    WiFi.node(WiFi_STA);
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED){
        gpio_set_level(GPIO_NUM_4, ledState);
        ledState = (ledState)? 0: 1;
        Serial.print(".");
        delay(1000);
    }

    Serial.println(WiFi.localIP());
    gpio_set_level(GPIO_NUM_4, 1);

}

void setup(){

}

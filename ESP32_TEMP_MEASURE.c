#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "WiFi.h"
#include <driver/adc.h>



void init_adc(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_08_0)
}


void temperature_measurement(void *p){
    int temperature = 0;

    while(1){
        temperature = adc1_get_raw(ADC1_CHANNEL_6);
        Serial.print(temperature);
        Serial.print
        vTaskDelay(2000 / portTICK_PERIOD_MS)
    }
}

int main(){
    Serial.begin(115200);
    init_adc();
    vTaskCreate(temperature_measurement, "temperature_measurement", 2048,NULL,5)

}

void setup(){
    main();
}
void loop(){}
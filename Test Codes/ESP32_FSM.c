//FSM

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    OFF,
    IDLE,
    MEASURING,
    DISPLAY,
    
} State;

typedef enum { 
    measure_temp,
    stop_temp_measure,
    display_temp,
    stop_temp_display,
    system_on,
    system_off,

} Events;

const char* state_names[] = {
    "OFF",
    "IDLE",
    "MEASURING",
    "DISPLAY"
};

const char* event_names[] = {
    "measure_temp",
    "stop_temp_measure",
    "display_temp",
    "stop_temp_display"
    "system_on",
    "system_off",

};

int main(){
    enum State current_state = IDLE;
    float temp = null; // 

    while(1){
        switch (current_state) {
        case OFF:
            if (Events == system_on) {
            printf("The system is now idle.");
            current_state = IDLE;
            }
            break;
        case IDLE:
            if (Events == measure_temp) {
            printf("The system is now measuring the room temperature.");
            current_state = MEASURING;
            }
            else if(Events == display_temp){
            printf("The system will now display the room temperature.");
            current_state = DISPLAY;
            }
            else if(Events == system_off){
            printf("The system will now shut down.");
            current_state = OFF;
            }
            break;
        case MEASURING:
            if (Events == stop_temp_measure) {
            printf("The system has stopped measuring the room temperature.");
            current_state = IDLE;
            }
            else if (Events == display_temp){
            printf("The system will now display the room temperature.");
            current_state = DISPLAY;
            }
            break;
        case DISPLAY:
            if (Events == stop_temp_display) {
            printf("The system has stopped displaying the room temperature.");
            current_state = IDLE;
            }
            else if (Events == system_off){
            printf("The system will now shut down.");
            current_state = OFF;
            }
            
            break;
        default:
            printf("Invalid state.\n");
            break;
        }
    }
}

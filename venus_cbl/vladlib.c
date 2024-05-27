#include "vladlib.h"
#include <libpynq.h>
#include <iic.h>
#include <stdio.h>
#include <stepper.h>

#define AS_SECONDS(time) (time / 1.0e8) // Timer runs at ~100MHZ
#define S0 IO_AR4
#define S1 IO_AR5
#define S2 IO_AR6
#define S3 IO_AR7
#define sensorOut IO_AR8

static uint32_t past_count = 0;
static uint32_t past_time = 0;

double getFreq(void) {
    uint32_t time = 0;
    uint32_t count = pulsecounter_get_count(PULSECOUNTER0, &time);
    if (past_time == 0) {
        past_time = time;
        past_count = count;
        return 0;
    }

    uint32_t delta_count = count - past_count;
    uint32_t delta_time = time - past_time;

    past_time = time;
    past_count = count;

    if (delta_time == 0) return 0;

    double frequency = delta_count / AS_SECONDS(delta_time);
    return frequency;
}

void readSensors(vl53x* sensorA, vl53x* sensorB) {
    uint32_t aDistance = tofReadDistance(sensorA);
    printf("A => %d mm ", aDistance);
    uint32_t bDistance = tofReadDistance(sensorB);
    printf("B => %d mm ", bDistance);

    // Setting RED (R) filtered photodiodes to be read
    gpio_set_level(S2, GPIO_LEVEL_LOW);
    gpio_set_level(S3, GPIO_LEVEL_LOW);
    sleep_msec(100); // Wait for a stable reading
    double redFrequency = getFreq();
    // printf("R = %.2f ", redFrequency);

    // Setting GREEN (G) filtered photodiodes to be read
    gpio_set_level(S2, GPIO_LEVEL_HIGH);
    gpio_set_level(S3, GPIO_LEVEL_HIGH);
    sleep_msec(100); // Wait for a stable reading
    double greenFrequency = getFreq();
    // printf("G = %.2f ", greenFrequency);

    // Setting BLUE (B) filtered photodiodes to be read
    gpio_set_level(S2, GPIO_LEVEL_LOW);
    gpio_set_level(S3, GPIO_LEVEL_HIGH);
    sleep_msec(100); // Wait for a stable reading
    double blueFrequency = getFreq();
    // printf("B = %.2f\n", blueFrequency);
    char color[10];
    if(redFrequency <= 5000 && greenFrequency <= 5000 && blueFrequency <= 5000){
        strcpy(color, "black");
    }else if(redFrequency >= 10000 && greenFrequency >= 10000 && blueFrequency >= 10000){
        strcpy(color, "white");
    }else if(redFrequency >= 6000 && greenFrequency <= 6000 && blueFrequency >= 6000){
        strcpy(color, "blue");
    }else if(redFrequency >= 6000 && greenFrequency >= 5000 && blueFrequency >= 5000 && blueFrequency <= 10000){
        strcpy(color, "green");
    }else if(redFrequency > greenFrequency && blueFrequency < redFrequency){
        strcpy(color, "red");
    }
    printf(" %s\n", color);
}


void move_right(){
    stepper_steps(1250, 0);
}

void move_left(){
    stepper_steps(0,1250);
}

#include <libpynq.h>
#include <iic.h>
#include "vl53l0x.h"
#include "vladlib.h"
#include <stdio.h>
#include <stepper.h>

#define S0 IO_AR4
#define S1 IO_AR5
#define S2 IO_AR6
#define S3 IO_AR7
#define sensorOut IO_AR8

int main(void)
{
    pynq_init();
	stepper_init();
	stepper_enable();
    // Init the IIC pins
    switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
    switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
    iic_init(IIC0);

    switchbox_set_pin(sensorOut, SWB_TIMER_IC0); // Pulsecounter 0 to sensorOut
    pulsecounter_init(PULSECOUNTER0);

    gpio_set_direction(S3, GPIO_DIR_OUTPUT);
    gpio_set_direction(S2, GPIO_DIR_OUTPUT);
    gpio_set_direction(S1, GPIO_DIR_OUTPUT);
    gpio_set_direction(S0, GPIO_DIR_OUTPUT);

    // Setting the sensorOut as an input
    gpio_set_direction(sensorOut, GPIO_DIR_INPUT);

    // Setting frequency scaling to 20%
    gpio_set_level(S0, GPIO_LEVEL_HIGH);
    gpio_set_level(S1, GPIO_LEVEL_LOW);

    // Use Rising edge triggering
    pulsecounter_set_edge(PULSECOUNTER0, GPIO_LEVEL_HIGH);
    pulsecounter_reset_count(PULSECOUNTER0);

    // range sensors initialization code

    int i;
    // Setup Sensor A
    printf("Initialising Sensor A:\n");

    // Change the Address of the VL53L0X
    uint8_t addrA = 0x69;
    i = tofSetAddress(IIC0, 0x29, addrA);
    printf("---Address Change: ");
    if (i != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    i = tofPing(IIC0, addrA);
    printf("---Sensor Ping: ");
    if (i != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    // Create a sensor struct
    vl53x sensorA;

    // Initialize the sensor
    i = tofInit(&sensorA, IIC0, addrA, 0);
    if (i != 0)
    {
        printf("---Init: Fail\n");
        return 1;
    }

    uint8_t model, revision;

    tofGetModel(&sensorA, &model, &revision);
    printf("---Model ID - %d\n", model);
    printf("---Revision ID - %d\n", revision);
    printf("---Init: Succes\n");
    fflush(NULL);

    printf("\n\nNow Power Sensor B!!\nPress \"Enter\" to continue...\n");
    getchar();

    // Setup Sensor B
    printf("Initialising Sensor B:\n");

    // Use the base addr of 0x29 for sensor B
    // It no longer conflicts with sensor A.
    uint8_t addrB = 0x29;
    i = tofPing(IIC0, addrB);
    printf("---Sensor Ping: ");
    if (i != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    // Create a sensor struct
    vl53x sensorB;

    // Initialize the sensor
    i = tofInit(&sensorB, IIC0, addrB, 0);
    if (i != 0)
    {
        printf("---Init: Fail\n");
        return 1;
    }

    tofGetModel(&sensorB, &model, &revision);
    printf("---Model ID - %d\n", model);
    printf("---Revision ID - %d\n", revision);
    printf("---Init: Succes\n");
    fflush(NULL); // Get some output even if the distance readings hang
    printf("\n");

    // initialization for the color sensor
    gpio_set_direction(S3, GPIO_DIR_OUTPUT);
    gpio_set_direction(S2, GPIO_DIR_OUTPUT);
    gpio_set_direction(S1, GPIO_DIR_OUTPUT);
    gpio_set_direction(S0, GPIO_DIR_OUTPUT);
    gpio_set_direction(sensorOut, GPIO_DIR_INPUT);

    gpio_set_level(S0, GPIO_LEVEL_HIGH);
    gpio_set_level(S1, GPIO_LEVEL_LOW);

    // setup no filter
    gpio_set_level(S2, GPIO_LEVEL_HIGH);
    gpio_set_level(S3, GPIO_LEVEL_HIGH);
	

	// actual movement of the robot

    while (1)
    {
        readSensors(&sensorA, &sensorB);
    }

    iic_destroy(IIC0);
    pulsecounter_destroy(PULSECOUNTER0);
    pynq_destroy();
    return EXIT_SUCCESS;
}

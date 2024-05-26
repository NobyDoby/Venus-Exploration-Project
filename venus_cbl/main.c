#include <libpynq.h>
#include <iic.h>
#include "vl53l0x.h"
#include <stdio.h>

#define AS_SECONDS(time) (time / 1.0e8) // Timer runs at ~100MHZ

#define S0 IO_AR4
#define S1 IO_AR5
#define S2 IO_AR6
#define S3 IO_AR7
#define sensorOut IO_AR8

#define AS_SECONDS(time) (time / 1.0e8) // Timer runs at ~100MHZ

uint32_t past_count = 0;
uint32_t past_time = 0;

double getFreq() {
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


int main(void)
{
	pynq_init();

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

	// uint32_t iDistance;
	// for (i=0; i<1200; i++)
	// {
	// 	iDistance = tofReadDistance(&sensorA);
	// 	printf("A => %dmm -- ", iDistance);
	// 	iDistance = tofReadDistance(&sensorB);
	// 	printf("B => %dmm\n", iDistance);
	// 	sleep_msec(100);
	// }

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

	

	uint32_t aDistance;
	uint32_t bDistance;
	
	while (1)
	{

		aDistance = tofReadDistance(&sensorA);
		printf("A => %d mm ", aDistance);
		bDistance = tofReadDistance(&sensorB);
		printf("B => %d mm ", bDistance);		
		

		// Setting RED (R) filtered photodiodes to be read
        gpio_set_level(S2, GPIO_LEVEL_LOW);
        gpio_set_level(S3, GPIO_LEVEL_LOW);
        sleep_msec(100); // Wait for a stable reading
        double redFrequency = getFreq();
        printf("R = %.2f ", redFrequency);

        // Setting GREEN (G) filtered photodiodes to be read
        gpio_set_level(S2, GPIO_LEVEL_HIGH);
        gpio_set_level(S3, GPIO_LEVEL_HIGH);
        sleep_msec(100); // Wait for a stable reading
        double greenFrequency = getFreq();
        printf("G = %.2f ", greenFrequency);

        // Setting BLUE (B) filtered photodiodes to be read
        gpio_set_level(S2, GPIO_LEVEL_LOW);
        gpio_set_level(S3, GPIO_LEVEL_HIGH);
        sleep_msec(100); // Wait for a stable reading
        double blueFrequency = getFreq();
        printf("B = %.2f\n", blueFrequency);
	}

	

	iic_destroy(IIC0);
	pulsecounter_destroy(PULSECOUNTER0);
	pynq_destroy();
	return EXIT_SUCCESS;
}
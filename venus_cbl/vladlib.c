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

#define UART_INDEX UART0    // Change this according to the UART index you’re using
#define MAX_BUFFER_SIZE 128 // Maximum size of the receive buffer

static uint32_t past_count = 0;
static uint32_t past_time = 0;

void uart_send_array(int uart, uint8_t *bytes, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        uart_send(uart, bytes[i]);
    }
}
void uart_receive_array(int uart, uint8_t *bytes, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        // Receive a byte from UART and store it in the array
        bytes[i] = uart_recv(uart);
        bytes[i + 1] = '\0';
    }
}
void com_from(int uart, uint8_t *buffer)
{
    uint32_t l;
    while (1)
    {
        // Check if UART has data available
        if (uart_has_data(uart))
        {
            // Receive a byte from UART
            uart_receive_array(uart, (uint8_t *)&l, sizeof(l));
            printf("Receiving length: %d \n", l);
            uart_receive_array(uart, buffer, l);
            break;
        }
    }
}

void com_to(int i, int j, int size, char index[10])
{

    // Uncomment the following code block to use payload-based sending

    // uint32_t payload = 10;

    // Uncomment the following lines to format and send a string via UART
    //   char buffer[128];
    //  uint32_t size = snprintf(buffer, 128, "Testing my test: %u\r\n", index);
    //  printf("%u\r\n", size);
    //  uart_send_array(UART0, (uint8_t *)&size, 4);
    //   uart_send_string(UART0, buffer);

    printf("Trying to send.\n");

    // //Uncomment the following lines to send payload size and value
    // uart_send(UART0, sizeof(payload));
    // uart_send(UART0, payload);

    char buffer[128];
    uint32_t l = snprintf(buffer, 128, "%d,%d,%d,%d,%s", i, j, 0, size, index);
    // l takes the length in bytes of the message
    uart_send_array(UART0, (uint8_t *)&l, sizeof(l));
    uart_send_array(UART0, (uint8_t *)&buffer, l);
    // Sends what is stored in buffer using function

    sleep_msec(250);
    // Uncomment the following line to increment the index in each loop iteration
    index++;
}

double getFreq(void)
{
    uint32_t time = 0;
    uint32_t count = pulsecounter_get_count(PULSECOUNTER0, &time);
    if (past_time == 0)
    {
        past_time = time;
        past_count = count;
        return 0;
    }

    uint32_t delta_count = count - past_count;
    uint32_t delta_time = time - past_time;

    past_time = time;
    past_count = count;

    if (delta_time == 0)
        return 0;

    double frequency = delta_count / AS_SECONDS(delta_time);
    return frequency;
}

void readSensors(vl53x *sensorA, vl53x *sensorB, uint32_t *aDistance, uint32_t *bDistance, char *color, int index)
{
    *aDistance = tofReadDistance(sensorA);
    // printf("A => %d mm ", *aDistance);
    *bDistance = tofReadDistance(sensorB);
    // printf("B => %d mm ", *bDistance);

    // Setting NO FILTER
    gpio_set_level(S2, GPIO_LEVEL_HIGH);
    gpio_set_level(S3, GPIO_LEVEL_LOW);
    sleep_msec(100);
    double Frequency = getFreq();
      printf("Freq = %.2f ", Frequency);

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

    if (Frequency > 17000)
    {
        // big block green check
        if (greenFrequency > redFrequency && greenFrequency > blueFrequency)
        {
            strcpy(color, "big green");
        }
        else
        {
            strcpy(color, "white");
        }
    }
    else if (Frequency < 8300 && redFrequency < 6000 && blueFrequency < 6000 && greenFrequency < 5000)
    {
        strcpy(color, "black");
    }

    else if (Frequency > 9000 && greenFrequency > blueFrequency)
    {
        strcpy(color, "green");
    } else if (Frequency > 9000 && blueFrequency > greenFrequency && blueFrequency > 6000)
    {
        strcpy(color, "blue");
    }
    else if (redFrequency > blueFrequency && redFrequency > greenFrequency)
    {
        strcpy(color, "red");
    }
   

    if (index == 1)
    {
        printf(" %s\n", color);
    }
}

void move_left()
{
    stepper_steps(1250, 0);
}

void move_right()
{
    stepper_steps(0, 1250);
}

void move_one_half_block()
{
    stepper_steps(200, 200);
}

void stop()
{
    stepper_steps(0, 0);
}

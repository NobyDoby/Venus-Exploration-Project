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

#define UART_INDEX UART0 // Change this according to the UART index you’re using
#define MAX_BUFFER_SIZE 128 // Maximum size of the receive buffer

void take_a_left(int *i, int *j, int aux) {
    switch(aux) {
        case 0:
            *i -= 2; 
            *j += 1; 
            break;
        case 1:
            *i-=1;
            *j-=2;
            break;
        case 2:
            *i+=2;
            *j-=1;
            break;
        case 3:
            *i+=1;
            *j+=2;
            break;
    }
}



int main(void)
{
    char map[50][50];
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            map[i][j] = '\0'; // Set each element to the null character
        }
    }
    pynq_init();
	stepper_init();
	stepper_enable();
    // initializations for communication from something
    // char buffer[MAX_BUFFER_SIZE];
     switchbox_set_pin(IO_AR0, SWB_UART0_RX);
    switchbox_set_pin(IO_AR1, SWB_UART0_TX);
    uart_init(UART_INDEX);
    uart_reset_fifos(UART_INDEX);


    // initializations for communication to something
     switchbox_set_pin(IO_AR0, SWB_UART0_RX);
    switchbox_set_pin(IO_AR1, SWB_UART0_TX);
    gpio_set_direction(IO_AR2, GPIO_DIR_INPUT);
    gpio_set_direction(IO_AR3, GPIO_DIR_INPUT);

    printf("AR2: %d\n", gpio_get_level(IO_AR0));
    printf("AR3: %d\n", gpio_get_level(IO_AR3));

    uart_init(UART0);
    uart_reset_fifos(UART0);


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
    // char vlad[10] = "hello";
    // while(1){
    //     com_to(0,0,0,vlad);
    // }
    int x;
    // Setup Sensor A
    printf("Initialising Sensor A:\n");

    // Change the Address of the VL53L0X
    uint8_t addrA = 0x69;
    x = tofSetAddress(IIC0, 0x29, addrA);
    printf("---Address Change: ");
    if (x != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    x = tofPing(IIC0, addrA);
    printf("---Sensor Ping: ");
    if (x != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    // Create a sensor struct
    vl53x sensorA;

    // Initialize the sensor
    x = tofInit(&sensorA, IIC0, addrA, 0);
    if (x != 0)
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

    printf("\n\nNow Power Sensor B!!\nInsert password to continue...\n");

    // while(1){
    //      com_from(UART_INDEX, (uint8_t *)&buffer);
    //      if(strcmp(buffer,"fuck_tue")==0){
    //         break;
    //      }
    // }

       getchar();

    // com_from(UART_INDEX, (uint8_t *)&buffer);
    // printf("%s", buffer);
    // Setup Sensor B
    printf("Initialising Sensor B:\n");

    // Use the base addr of 0x29 for sensor B
    // It no longer conflicts with sensor A.
    uint8_t addrB = 0x29;
    x = tofPing(IIC0, addrB);
    printf("---Sensor Ping: ");
    if (x != 0)
    {
        printf("Fail\n");
        return 1;
    }
    printf("Succes\n");

    // Create a sensor struct
    vl53x sensorB;

    // Initialize the sensor
    x = tofInit(&sensorB, IIC0, addrB, 0);
    if (x != 0)
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

	// actual movement of the robot
    uint32_t aDistance = 0;
    uint32_t bDistance = 0;
    char color[10];
     int size = 1;
    

    // while(1){
    //     com_from(UART_INDEX, (uint8_t *)&buffer);
    //     printf("\"%s\" \n", buffer);
    // }

    // com_to("Congratulations!");

//    code for color testing
   
// // while(1){
//      readSensors(&sensorA, &sensorB, &aDistance, &bDistance, color, 1);
// }
     int k = 0;
      int i = 25, j = 25;
// //    char *case[] = {"+x", "+y", "-x", "-y"};
   int aux = 0;
   
  
    while(1){
    readSensors(&sensorA, &sensorB, &aDistance, &bDistance, color, 0);
    stepper_set_speed(60000,60000);
   move_one_half_block();
   k++;
   if(k > 1){
    k= 0;
    switch(aux){
        case 0:
        i++;
        break;
        case 1:
        j++;
        break;
        case 2:
        i--;
        break;
        case 3:
        j--;
        break;
    }
   }
    if(aDistance > 300){
    readSensors(&sensorA, &sensorB, &aDistance, &bDistance, color, 0);
    if(strcmp(color,"black") == 0){
         stop();
         printf("Hitting that border on coordinate %d %d\n", i, j);
         sleep_msec(2000);
        stepper_steps(-600, -600);
        map[i][j] = 'B';
        sleep_msec(2000);
        move_left();
        sleep_msec(2000);
         take_a_left(&i,&j,aux);
        if(aux != 3){
            aux++;
        }else{
            aux = 0;
        }
    }
} else if(aDistance < 300){
    if(aDistance < 100 && bDistance < 200){
        stop();
          printf("Hitting that mountain on coordinate %d %d\n", i, j);
         sleep_msec(2000);
         stepper_steps(-600, -600);
         map[i][j] = 'M';
        sleep_msec(2000);
        move_left();
        sleep_msec(2000);
        take_a_left(&i,&j,aux);
         if(aux != 3){
            aux++;
        }else{
            aux = 0;
        }
         }else if(aDistance < 60){
        stop();
         readSensors(&sensorA, &sensorB, &aDistance, &bDistance, color, 1);
        sleep_msec(2000);
        // printf("%s on %d %d\n",color, i, j);
        if(strcmp(color, "big green") == 0){
            size = 2;
        }else{
            size = 1;
        }
       com_to(i,j,size,color);
        sleep_msec(2000);
        map[i][j] = 'R';
        stepper_steps(-600, -600);
        sleep_msec(2000);
        move_left();
        sleep_msec(2000);
         take_a_left(&i,&j,aux);
        if(aux != 3){
            aux++;
        }else{
            aux = 0;
        }
       
    }
}
    }
    for(int a = 0; a < 50; a++){
        for(int b = 0; b < 50; b++){
            if(map[a][b] == '\0'){
                printf("0");
            }else{
            printf("%c", map[a][b]);
        }
        }
        printf("\n");
    }

    iic_destroy(IIC0);
    pulsecounter_destroy(PULSECOUNTER0);
    pynq_destroy();
    return EXIT_SUCCESS;
}


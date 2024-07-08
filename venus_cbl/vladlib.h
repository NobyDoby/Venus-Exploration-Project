#ifndef VLADLIB_H
#define VLADLIB_H

#include <stdint.h>
#include "vl53l0x.h"
#include <stepper.h>

double getFreq(void);
void readSensors(vl53x* sensorA, vl53x* sensorB, uint32_t* aDistance, uint32_t* bDistance, char * color, int index);
void uart_receive_array(int uart, uint8_t *bytes, uint32_t length);
void com_from(int uart, uint8_t *buffer);
void move_right(void);
void move_left(void);
void move_one_half_block(void);
void stop(void);
void com_to(int i, int j, int size, char index[10]);
void uart_send_array(int uart, uint8_t *bytes, uint32_t length);
#endif // VLADLIB_H

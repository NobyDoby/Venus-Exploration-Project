#ifndef VLADLIB_H
#define VLADLIB_H

#include <stdint.h>
#include "vl53l0x.h"
#include <stepper.h>

double getFreq(void);
void readSensors(vl53x* sensorA, vl53x* sensorB);
void move_right(void);
void move_left(void);


#endif // VLADLIB_H

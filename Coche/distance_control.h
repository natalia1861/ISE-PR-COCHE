#ifndef __DISTANCE_CONTROL_H
#define __DISTANCE_CONTROL_H

#include <stdint.h>

extern uint16_t distancia;

void Init_DistanceControl (void);
void Stop_DistanceControl (void);
void Init_SensorDistancia (void);

#endif

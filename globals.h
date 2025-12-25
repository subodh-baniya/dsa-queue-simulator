
#ifndef GLOBALS_H
#define GLOBALS_H

#include "vehicle.h"

// Declaring all global variables 
extern RoadData roads[4];
extern TransitionVehicle transitions[MAX_QUEUE];
extern int transitionCount;
extern int currentGreen;
extern int lightState;
extern Uint32 lightTimer;
extern Uint32 dynamicGreenTime;
extern Uint32 redPhaseStartTime;
extern int vehiclesToServe;
extern int vehiclesServed;
extern const char* basedir;
extern const char* files[4];
extern TrafficPriorityQueue trafficQueue;

#endif
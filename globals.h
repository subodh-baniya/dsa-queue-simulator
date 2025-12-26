#ifndef GLOBALS_H
#define GLOBALS_H

#include "vehicle.h"

extern RoadData roads[4];    // 4 roads, each with 3 lanes
extern TransitionVehicle transitions[MAX_QUEUE]; // Vehicles changing roads
extern int transitionCount;         // Current number of transitioning vehicles

extern int currentGreen;            // Which road has green 
extern int lightState;              // GREEN_LIGHT or RED_PHASE
extern Uint32 lightTimer;           // Timer for light changes
extern Uint32 dynamicGreenTime;     // How long green lasts 
extern Uint32 redPhaseStartTime;    // When red phase started
extern int vehiclesToServe;         // Vehicles to serve this green phase
extern int vehiclesServed;          // Vehicles already served this phase

// File paths for vehicle data
extern const char* basedir;         
extern const char* files[4];       

// Priority queue 
extern TrafficPriorityQueue trafficQueue; // Decides which road gets green next

#endif
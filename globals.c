#include "globals.h"
#include "constants.h"

// Declaring all global variables
RoadData roads[4];
TransitionVehicle transitions[MAX_QUEUE];
int transitionCount = 0;
int currentGreen = 0;
int lightState = GREEN_LIGHT;
Uint32 lightTimer = 0;
Uint32 dynamicGreenTime = 0;
Uint32 redPhaseStartTime = 0;
int vehiclesToServe = 0;
int vehiclesServed = 0;

TrafficPriorityQueue trafficQueue;

const char* basedir = "C:\\TrafficShared\\";
const char* files[4] = {
    "C:\\TrafficShared\\lanea.txt",
    "C:\\TrafficShared\\laneb.txt",
    "C:\\TrafficShared\\lanec.txt",
    "C:\\TrafficShared\\laned.txt"
};
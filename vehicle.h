// vehicle.h
#ifndef VEHICLE_H
#define VEHICLE_H

#include <SDL.h>
#include <math.h>
#include <string.h>
#include "constants.h"

// Vehicle structure
typedef struct {
    int id;
    float x, y;
    int fromRoad;
    int isStopped;
    char name[NAME_MAX];
} Vehicle;

// Transition vehicle structure
typedef struct {
    Vehicle v;
    int targetRoad;
    int waitingTime;
} TransitionVehicle;

// Circular queue for lanes
typedef struct {
    Vehicle data[MAX_QUEUE];
    int front, rear, count;
} Lane;

// Road data structure
typedef struct {
    Lane L1;
    Lane L2;
    Lane L3;
} RoadData;

void initLane(Lane* l);
int enqueue(Lane* l, Vehicle v);
int dequeue(Lane* l, Vehicle* out);
Vehicle* getLaneVehicle(Lane* l, int index);
float distance(float x1, float y1, float x2, float y2);
int getOppositeRoad(int road);

#endif
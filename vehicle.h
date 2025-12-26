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
    char name[NAME_MAX];
    int isStopped;
    int isTransitioning;
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
    int maxAllowed;
} Lane;

// Road data structure
typedef struct {
    Lane L1;
    Lane L2;
    Lane L3;
} RoadData;

//Priority Queue element structure
typedef struct {
    int roadIndex;
    int priority;
    Uint32 lastServedTime;
} PriorityQueueElement;

// Priority Queue structure
typedef struct {
    PriorityQueueElement elements[4];
    int size;
} TrafficPriorityQueue;

void initLane(Lane* l);
void initLaneWithMax(Lane* l, int maxAllowed);
int enqueue(Lane* l, Vehicle v);
int dequeue(Lane* l, Vehicle* out);
Vehicle* getLaneVehicle(Lane* l, int index);
float distance(float x1, float y1, float x2, float y2);
int getOppositeRoad(int road);

#endif
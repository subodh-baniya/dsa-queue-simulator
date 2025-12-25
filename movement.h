
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "vehicle.h"
#include "geometry.h"

int checkTooCloseInLane(Lane* l, float x, float y, int skipIndex);
float getDistanceToVehicleAhead(Lane* L, int road, int vehicleIndex);
void moveLaneL3(Lane* L, int road);
void moveLaneTowardCenter(Lane* L, int road);
void addTransition(Vehicle v, int targetRoad);
void moveTransitions();
void cleanupStuckTransitions();
void calculateVehiclesToServe();
void initPriorityQueue(TrafficPriorityQueue* pq);
void updateAllPriorities(TrafficPriorityQueue* pq);
int getNextRoadToServe(TrafficPriorityQueue* pq, Uint32 currentTime);

#endif
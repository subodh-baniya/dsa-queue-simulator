
#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "vehicle.h"
#include "geometry.h"

int checkTooCloseInLane(Lane* l, float x, float y, int skipIndex); // Check if a position is too close to other vehicles
float getDistanceToVehicleAhead(Lane* L, int road, int vehicleIndex); // Distance to next vehicle ahead for stopping
void moveLaneL3(Lane* L, int road); // Move L3 vehicles
void moveLaneTowardCenter(Lane* L, int road); // Move L1/L2 vehicles toward intersection
void addTransition(Vehicle v, int targetRoad); // Add a vehicle to transition between roads
void moveTransitions(); // Update all transitioning vehicles
void cleanupStuckTransitions(); // Clean up vehicles stuck in transition too long
void calculateVehiclesToServe();// Calculate how many vehicles to serve during green light

//Priority queue functions
void initPriorityQueue(TrafficPriorityQueue* pq);
void updateAllPriorities(TrafficPriorityQueue* pq);
int getNextRoadToServe(TrafficPriorityQueue* pq, Uint32 currentTime);

#endif
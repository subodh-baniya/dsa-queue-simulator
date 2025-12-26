
#include "vehicle.h"
#include "globals.h"

// Initialize lane with no max limit
void initLane(Lane* l) {
    l->front = 0;
    l->rear = -1;
    l->count = 0;
    l->maxAllowed = 0;
}

// Initialize a lane with maximum capacity limit
void initLaneWithMax(Lane* l, int maxAllowed) {
    l->front = 0;
    l->rear = -1;
    l->count = 0;
    l->maxAllowed = maxAllowed;
}

// Add a vehicle to the end of the lane queue
// Returns 1 if successful, 0 if failed
int enqueue(Lane* l, Vehicle v) {
    if (l->count >= MAX_QUEUE) return 0;
    if (l->maxAllowed > 0 && l->count >= l->maxAllowed) return 0;
    l->rear = (l->rear + 1) % MAX_QUEUE;
    l->data[l->rear] = v;
    l->count++;
    return 1;
}

// Remove a vehicle from the front of the lane queue
// Returns 1 if successful, 0 if queue is empty
int dequeue(Lane* l, Vehicle* out) {
    if (l->count == 0) return 0;
    *out = l->data[l->front];
    l->front = (l->front + 1) % MAX_QUEUE;
    l->count--;
    return 1;
}

// Get pointer to vehicle at specific index in lane
// Returns NULL if index is invalid
Vehicle* getLaneVehicle(Lane* l, int index) {
    if (index < 0 || index >= l->count) return NULL;
    int idx = (l->front + index) % MAX_QUEUE;
    return &l->data[idx];
}

// Calculate Euclidean distance between two points
// Used for checking vehicle spacing and collisions
float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// Get the opposite road index
// Used for determining where L2 vehicles should transition to
int getOppositeRoad(int road) {
    switch (road) {
    case 0: return 2;
    case 1: return 3;
    case 2: return 0;
    case 3: return 1;
    }
    return 0;
}
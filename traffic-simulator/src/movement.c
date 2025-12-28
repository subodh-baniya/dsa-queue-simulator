#define _CRT_SECURE_NO_WARNINGS
#include "./headers/movement.h"
#include <stdio.h>
#include "./headers/globals.h"

// Check if position is too close to any vehicle in lane
// Returns 1 if too close, 0 otherwise
int checkTooCloseInLane(Lane* l, float x, float y, int skipIndex) {
    for (int i = 0; i < l->count; i++) {
        if (i == skipIndex) continue; 
        Vehicle* other = getLaneVehicle(l, i);
        if (!other) continue;
        float dist = distance(x, y, other->x, other->y);
        if (dist < MIN_SPACING) return 1; 
    }
    return 0; 
}

// Calculate distance to next vehicle ahead in same lane
// Returns distance in pixels, or 999999 if no vehicle ahead
float getDistanceToVehicleAhead(Lane* L, int road, int vehicleIndex) {
    Vehicle* current = getLaneVehicle(L, vehicleIndex);
    if (!current) return 999999.0f;

    float minDist = 999999.0f; 

    // Check all vehicles ahead of current one
    for (int i = vehicleIndex + 1; i < L->count; i++) {
        Vehicle* ahead = getLaneVehicle(L, i);
        if (!ahead) continue;

        float distAlongRoad = 0.0f;
        // Calculate distance along road direction
        switch (road) {
        case 0: distAlongRoad = ahead->y - current->y; break;  // North to South
        case 1: distAlongRoad = current->x - ahead->x; break;  // East to West  
        case 2: distAlongRoad = current->y - ahead->y; break;  // South to North
        case 3: distAlongRoad = ahead->x - current->x; break;  // West to East
        }

        // Only consider vehicles that are ahead
        if (distAlongRoad > 0 && distAlongRoad < minDist) {
            minDist = distAlongRoad;
        }
    }

    return minDist;
}

// Move L3 lane vehicles 
void moveLaneL3(Lane* L, int road) {
    float dx, dy;
    l3_move_vector(road, &dx, &dy);  // Get movement direction

    int i = 0;
    while (i < L->count) {
        Vehicle* v = getLaneVehicle(L, i);
        if (!v) {
            i++;
            continue;
        }

        float newx = v->x + dx;
        float newy = v->y + dy;

        // Remove vehicle if it goes off screen
        if (newx < -100 || newx > SCREEN_W + 100 || newy < -100 || newy > SCREEN_H + 100) {
            Vehicle temp;
            dequeue(L, &temp);
            continue;  
        }

        // Check spacing with other vehicles in same lane
        int canMove = 1;
        for (int j = 0; j < L->count; j++) {
            if (i == j) continue;
            Vehicle* other = getLaneVehicle(L, j);
            if (!other) continue;

            float distToOther;
            switch (road) {
            case 0: distToOther = v->y - other->y; break;  // North road
            case 1: distToOther = other->x - v->x; break;  // East road
            case 2: distToOther = other->y - v->y; break;  // South road
            case 3: distToOther = v->x - other->x; break;  // West road
            }

            // If vehicle ahead is too close, can't move
            if (distToOther > 0 && distToOther < MIN_SPACING) {
                canMove = 0;
                break;
            }
        }

        if (canMove) {
            v->x = newx;
            v->y = newy;
            v->isStopped = 0;
        }
        else {
            v->isStopped = 1;  // Stop due to traffic
        }

        i++;
    }
}

// Move L1/L2 lane vehicles toward intersection center
void moveLaneTowardCenter(Lane* L, int road) {
    float mvx = 0.0f, mvy = 0.0f;
    // Set movement direction based on road
    if (road == 0) { mvy = VEHICLE_SPEED; }      // North: move down
    else if (road == 1) { mvx = -VEHICLE_SPEED; } // East: move left
    else if (road == 2) { mvy = -VEHICLE_SPEED; } // South: move up
    else { mvx = VEHICLE_SPEED; }                 // West: move right

    // Process from back to front
    for (int i = L->count - 1; i >= 0; i--) {
        Vehicle* v = getLaneVehicle(L, i);
        if (!v) continue;

        // Calculate distance to intersection stop line
        float cx = SCREEN_W / 2.0f;
        float cy = SCREEN_H / 2.0f;
        float distToIntersection;

        if (road == 0) distToIntersection = cy - ROAD_W / 2.0f - v->y;
        else if (road == 1) distToIntersection = v->x - (cx + ROAD_W / 2.0f);
        else if (road == 2) distToIntersection = v->y - (cy + ROAD_W / 2.0f);
        else distToIntersection = cx - ROAD_W / 2.0f - v->x;

        // Check if should stop at traffic light
        int shouldStopAtLight = (lightState == RED_PHASE || currentGreen != road);

        // Stop if at red light and close to intersection
        if (shouldStopAtLight && distToIntersection < STOPPING_DISTANCE && distToIntersection > 0) {
            v->isStopped = 1;
            continue;
        }

        // Check distance to vehicle ahead
        float distToAhead = getDistanceToVehicleAhead(L, road, i);

        // Stop if too close to vehicle ahead
        if (distToAhead < STOPPING_DISTANCE) {
            if (i + 1 < L->count) {
                Vehicle* ahead = getLaneVehicle(L, i + 1);
                if (ahead && ahead->isStopped) {
                    v->isStopped = 1;  // Stop because ahead vehicle is stopped
                    continue;
                }
            }

            if (distToAhead < MIN_FRONT_SPACING) {
                v->isStopped = 1;  // Stop for safe distance
                continue;
            }
        }

        // Calculate new position
        float newx = v->x + mvx;
        float newy = v->y + mvy;

        // Move if not too close to other vehicles
        if (!checkTooCloseInLane(L, newx, newy, i)) {
            v->x = newx;
            v->y = newy;
            v->isStopped = 0;
        }
        else {
            v->isStopped = 1;  // Stop due to crowding
        }

        // Remove vehicle if it goes far off screen
        if (v->x < -100 || v->x > SCREEN_W + 100 || v->y < -100 || v->y > SCREEN_H + 100) {
            for (int j = i; j < L->count - 1; j++) {
                L->data[(L->front + j) % MAX_QUEUE] = L->data[(L->front + j + 1) % MAX_QUEUE];
            }
            L->count--;
        }
    }
}

// Add vehicle to transition queue
void addTransition(Vehicle v, int targetRoad) {
    if (transitionCount >= MAX_QUEUE) return;  // Queue full

    v.isStopped = 0;
    v.isTransitioning = 1;
    transitions[transitionCount].v = v;
    transitions[transitionCount].targetRoad = targetRoad;
    transitions[transitionCount].waitingTime = 0;
    transitionCount++;
}

// Move transitioning vehicles toward target road
void moveTransitions() {
    // Sort by waiting time
    for (int i = 0; i < transitionCount - 1; i++) {
        for (int j = i + 1; j < transitionCount; j++) {
            if (transitions[j].waitingTime > transitions[i].waitingTime) {
                TransitionVehicle temp = transitions[i];
                transitions[i] = transitions[j];
                transitions[j] = temp;
            }
        }
    }

    // Move each transitioning vehicle
    for (int i = 0; i < transitionCount; i++) {
        TransitionVehicle* tv = &transitions[i];
        tv->waitingTime++;
        tv->v.isStopped = 0;

        // Get target position
        float tx, ty;
        intersection_lane_center(tv->targetRoad, 3, &tx, &ty);
        float dx = tx - tv->v.x;
        float dy = ty - tv->v.y;
        float dist = sqrtf(dx * dx + dy * dy);

        float speed = VEHICLE_SPEED;

        if (dist < speed) {
            tv->v.x = tx;
            tv->v.y = ty;

            tv->v.isTransitioning = 0;
            tv->v.fromRoad = tv->targetRoad;

            // Try to join target lane if space available
            if (!checkTooCloseInLane(&roads[tv->targetRoad].L3, tx, ty, -1)) {
                enqueue(&roads[tv->targetRoad].L3, tv->v);

                // Remove from transitions array
                for (int j = i; j < transitionCount - 1; j++) {
                    transitions[j] = transitions[j + 1];
                }
                transitionCount--;
                i--;
            }
            else {
                tv->v.isStopped = 1;
                if (tv->waitingTime > 50) { 
                    enqueue(&roads[tv->targetRoad].L3, tv->v);

                    for (int j = i; j < transitionCount - 1; j++) {
                        transitions[j] = transitions[j + 1];
                    }
                    transitionCount--;
                    i--;
                }
            }
        }
        else {
            // Move toward target
            int canMove = 1;

            // Check spacing with other transitioning vehicles
            for (int j = 0; j < transitionCount; j++) {
                if (i == j) continue;
                float otherDist = distance(tv->v.x, tv->v.y,
                    transitions[j].v.x, transitions[j].v.y);
                if (otherDist < MIN_FRONT_SPACING * 1.5f) {
                    if (transitions[j].waitingTime >= tv->waitingTime) {
                        canMove = 0;  
                        break;
                    }
                }
            }

            if (canMove) {
                float newx = tv->v.x + speed * dx / dist;
                float newy = tv->v.y + speed * dy / dist;

                if (fabs(dx) < fabs(newx - tv->v.x)) newx = tx;
                if (fabs(dy) < fabs(newy - tv->v.y)) newy = ty;

                tv->v.x = newx;
                tv->v.y = newy;
                tv->v.isStopped = 0;
            }
            else {
                tv->v.isStopped = 1; 
            }
        }
    }
}

// Clean up vehicles stuck in transition too long
void cleanupStuckTransitions() {
    static Uint32 lastCleanup = 0;
    Uint32 now = SDL_GetTicks();

    // Check every 5 seconds
    if (now - lastCleanup >= 5000) {
        for (int i = 0; i < transitionCount; i++) {
            if (transitions[i].waitingTime > 150) { 
                for (int j = i; j < transitionCount - 1; j++) {
                    transitions[j] = transitions[j + 1];
                }
                transitionCount--;
                i--;
            }
        }
        lastCleanup = now;
    }
}

// Calculate how many vehicles to serve in current green phase
void calculateVehiclesToServe() {
    int totalWaiting = roads[currentGreen].L1.count + roads[currentGreen].L2.count;

    vehiclesToServe = totalWaiting;
    if (vehiclesToServe < 1) vehiclesToServe = 1; 

    dynamicGreenTime = vehiclesToServe * TIME_PER_VEHICLE;

}

// Initialize priority queue for traffic lights
void initPriorityQueue(TrafficPriorityQueue* pq) {
    pq->size = 4;
    for (int i = 0; i < 4; i++) {
        pq->elements[i].roadIndex = i;
        pq->elements[i].priority = NORMAL_PRIORITY;
        pq->elements[i].lastServedTime = 0;
    }
}

// Update priorities based on L2 vehicle counts
void updateAllPriorities(TrafficPriorityQueue* pq) {
    for (int i = 0; i < pq->size; i++) {
        int roadIdx = pq->elements[i].roadIndex;

        // High priority if L2 count > threshold
        if (roads[roadIdx].L2.count > PRIORITY_THRESHOLD) {
            if (pq->elements[i].priority != HIGH_PRIORITY) {
                pq->elements[i].priority = HIGH_PRIORITY;
            }
        }
        // Back to normal if L2 count < reset threshold
        else if (roads[roadIdx].L2.count < PRIORITY_RESET) {
            if (pq->elements[i].priority != NORMAL_PRIORITY) {
                pq->elements[i].priority = NORMAL_PRIORITY;
            }
        }
    }
}

// Determine which road should get green light next
int getNextRoadToServe(TrafficPriorityQueue* pq, Uint32 currentTime) {
    int bestRoad = -1;
    int highestPriority = -1;
    Uint32 oldestTime = currentTime;

    // Find road with highest priority that has waiting vehicles
    for (int i = 0; i < pq->size; i++) {
        int roadIdx = pq->elements[i].roadIndex;
        int priority = pq->elements[i].priority;

        // Skip roads with no waiting vehicles
        if (roads[roadIdx].L1.count == 0 && roads[roadIdx].L2.count == 0) {
            continue;
        }

        // Higher priority road wins
        if (priority > highestPriority) {
            highestPriority = priority;
            bestRoad = roadIdx;
            oldestTime = pq->elements[i].lastServedTime;
        }
        // If same priority, older last-served time wins
        else if (priority == highestPriority) {
            if (pq->elements[i].lastServedTime < oldestTime) {
                bestRoad = roadIdx;
                oldestTime = pq->elements[i].lastServedTime;
            }
        }
    }

    // If no road has waiting vehicles, use round-robin
    if (bestRoad == -1) {
        bestRoad = (currentGreen + 1) % 4;
    }

    // Update last served time for chosen road
    for (int i = 0; i < pq->size; i++) {
        if (pq->elements[i].roadIndex == bestRoad) {
            pq->elements[i].lastServedTime = currentTime;
            break;
        }
    }

    return bestRoad;
}
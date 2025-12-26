
#define _CRT_SECURE_NO_WARNINGS
#include "movement.h"
#include <stdio.h>
#include "globals.h"

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

float getDistanceToVehicleAhead(Lane* L, int road, int vehicleIndex) {
    Vehicle* current = getLaneVehicle(L, vehicleIndex);
    if (!current) return 999999.0f;

    float minDist = 999999.0f;

    for (int i = vehicleIndex + 1; i < L->count; i++) {
        Vehicle* ahead = getLaneVehicle(L, i);
        if (!ahead) continue;

        float distAlongRoad = 0.0f;
        switch (road) {
        case 0: distAlongRoad = ahead->y - current->y; break;
        case 1: distAlongRoad = current->x - ahead->x; break;
        case 2: distAlongRoad = current->y - ahead->y; break;
        case 3: distAlongRoad = ahead->x - current->x; break;
        }

        if (distAlongRoad > 0 && distAlongRoad < minDist) {
            minDist = distAlongRoad;
        }
    }

    return minDist;
}

void moveLaneL3(Lane* L, int road) {
    float dx, dy;
    l3_move_vector(road, &dx, &dy);

    int i = 0;
    while (i < L->count) {
        Vehicle* v = getLaneVehicle(L, i);
        if (!v) {
            i++;
            continue;
        }

        float newx = v->x + dx;
        float newy = v->y + dy;

        if (newx < -100 || newx > SCREEN_W + 100 || newy < -100 || newy > SCREEN_H + 100) {
            Vehicle temp;
            dequeue(L, &temp);
            continue;
        }

        int canMove = 1;
        for (int j = 0; j < L->count; j++) {
            if (i == j) continue;
            Vehicle* other = getLaneVehicle(L, j);
            if (!other) continue;

            float distToOther;
            switch (road) {
            case 0: distToOther = v->y - other->y; break;
            case 1: distToOther = other->x - v->x; break;
            case 2: distToOther = other->y - v->y; break;
            case 3: distToOther = v->x - other->x; break;
            }

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
            v->isStopped = 1;
        }

        i++;
    }
}

void moveLaneTowardCenter(Lane* L, int road) {
    float mvx = 0.0f, mvy = 0.0f;
    if (road == 0) { mvy = VEHICLE_SPEED; }
    else if (road == 1) { mvx = -VEHICLE_SPEED; }
    else if (road == 2) { mvy = -VEHICLE_SPEED; }
    else { mvx = VEHICLE_SPEED; }

    for (int i = L->count - 1; i >= 0; i--) {
        Vehicle* v = getLaneVehicle(L, i);
        if (!v) continue;

        float cx = SCREEN_W / 2.0f;
        float cy = SCREEN_H / 2.0f;
        float distToIntersection;

        if (road == 0) distToIntersection = cy - ROAD_W / 2.0f - v->y;
        else if (road == 1) distToIntersection = v->x - (cx + ROAD_W / 2.0f);
        else if (road == 2) distToIntersection = v->y - (cy + ROAD_W / 2.0f);
        else distToIntersection = cx - ROAD_W / 2.0f - v->x;

        int shouldStopAtLight = (lightState == RED_PHASE || currentGreen != road);

        if (shouldStopAtLight && distToIntersection < STOPPING_DISTANCE && distToIntersection > 0) {
            v->isStopped = 1;
            continue;
        }

        float distToAhead = getDistanceToVehicleAhead(L, road, i);

        if (distToAhead < STOPPING_DISTANCE) {
            if (i + 1 < L->count) {
                Vehicle* ahead = getLaneVehicle(L, i + 1);
                if (ahead && ahead->isStopped) {
                    v->isStopped = 1;
                    continue;
                }
            }

            if (distToAhead < MIN_FRONT_SPACING) {
                v->isStopped = 1;
                continue;
            }
        }

        float newx = v->x + mvx;
        float newy = v->y + mvy;

        if (!checkTooCloseInLane(L, newx, newy, i)) {
            v->x = newx;
            v->y = newy;
            v->isStopped = 0;
        }
        else {
            v->isStopped = 1;
        }

        if (v->x < -100 || v->x > SCREEN_W + 100 || v->y < -100 || v->y > SCREEN_H + 100) {
            for (int j = i; j < L->count - 1; j++) {
                L->data[(L->front + j) % MAX_QUEUE] = L->data[(L->front + j + 1) % MAX_QUEUE];
            }
            L->count--;
        }
    }
}

// transition addition
void addTransition(Vehicle v, int targetRoad) {
    if (transitionCount >= MAX_QUEUE) return;
    v.isStopped = 0;
    v.isTransitioning = 1;
    transitions[transitionCount].v = v;
    transitions[transitionCount].targetRoad = targetRoad;
    transitions[transitionCount].waitingTime = 0;
    transitionCount++;
}

// movement in transition
void moveTransitions() {
    for (int i = 0; i < transitionCount - 1; i++) {
        for (int j = i + 1; j < transitionCount; j++) {
            if (transitions[j].waitingTime > transitions[i].waitingTime) {
                TransitionVehicle temp = transitions[i];
                transitions[i] = transitions[j];
                transitions[j] = temp;
            }
        }
    }

    for (int i = 0; i < transitionCount; i++) {
        TransitionVehicle* tv = &transitions[i];
        tv->waitingTime++;
        tv->v.isStopped = 0;

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

            if (!checkTooCloseInLane(&roads[tv->targetRoad].L3, tx, ty, -1)) {
                enqueue(&roads[tv->targetRoad].L3, tv->v);

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
            int canMove = 1;

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

void cleanupStuckTransitions() {
    static Uint32 lastCleanup = 0;
    Uint32 now = SDL_GetTicks();

    if (now - lastCleanup >= 5000) {
        for (int i = 0; i < transitionCount; i++) {
            if (transitions[i].waitingTime > 150) {
                printf("Removing stuck transition vehicle %d (waited %d cycles)\n",
                    transitions[i].v.id, transitions[i].waitingTime);

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

void calculateVehiclesToServe() {
    int totalWaiting = roads[currentGreen].L1.count + roads[currentGreen].L2.count;

    vehiclesToServe = totalWaiting;
    if (vehiclesToServe < 1) vehiclesToServe = 1;

    dynamicGreenTime = vehiclesToServe * TIME_PER_VEHICLE;

}

void initPriorityQueue(TrafficPriorityQueue* pq) {
    pq->size = 4;
    for (int i = 0; i < 4; i++) {
        pq->elements[i].roadIndex = i;
        pq->elements[i].priority = NORMAL_PRIORITY;
        pq->elements[i].lastServedTime = 0;
    }
}

void updateAllPriorities(TrafficPriorityQueue* pq) {
    for (int i = 0; i < pq->size; i++) {
        int roadIdx = pq->elements[i].roadIndex;

        if (roads[roadIdx].L2.count > PRIORITY_THRESHOLD) {
            if (pq->elements[i].priority != HIGH_PRIORITY) {
                pq->elements[i].priority = HIGH_PRIORITY;
                printf("Road %d escalated to HIGH PRIORITY (L2 count: %d)\n",
                    roadIdx, roads[roadIdx].L2.count);
            }
        }
        else if (roads[roadIdx].L2.count < PRIORITY_RESET) {
            if (pq->elements[i].priority != NORMAL_PRIORITY) {
                pq->elements[i].priority = NORMAL_PRIORITY;
                printf("Road %d returned to NORMAL PRIORITY (L2 count: %d)\n",
                    roadIdx, roads[roadIdx].L2.count);
            }
        }
       
    }
}

int getNextRoadToServe(TrafficPriorityQueue* pq, Uint32 currentTime) {
    int bestRoad = -1;
    int highestPriority = -1;
    Uint32 oldestTime = currentTime;

    for (int i = 0; i < pq->size; i++) {
        int roadIdx = pq->elements[i].roadIndex;
        int priority = pq->elements[i].priority;

        if (roads[roadIdx].L1.count == 0 && roads[roadIdx].L2.count == 0) {
            continue;
        }

        if (priority > highestPriority) {
            highestPriority = priority;
            bestRoad = roadIdx;
            oldestTime = pq->elements[i].lastServedTime;
        }
        else if (priority == highestPriority) {
            if (pq->elements[i].lastServedTime < oldestTime) {
                bestRoad = roadIdx;
                oldestTime = pq->elements[i].lastServedTime;
            }
        }
    }

    if (bestRoad == -1) {
        bestRoad = (currentGreen + 1) % 4;
    }

    for (int i = 0; i < pq->size; i++) {
        if (pq->elements[i].roadIndex == bestRoad) {
            pq->elements[i].lastServedTime = currentTime;
            break;
        }
    }

    return bestRoad;
}
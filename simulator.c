#define _CRT_SECURE_NO_WARNINGS 
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <string.h>

#define SCREEN_W 900
#define SCREEN_H 900
#define ROAD_W 200
#define LANE_W (ROAD_W/3.0f)
#define MAX_QUEUE 50
#define VEHICLE_SPEED 2.0f
#define VEHICLE_SIZE 12
#define STOPPING_DISTANCE 30.0f
#define MIN_SPACING 20.0f
#define MIN_FRONT_SPACING 25.0f
#define sleep_ms(x) Sleep(x)
#define GREEN_LIGHT 0
#define RED_PHASE 1
#define RED_PHASE_DURATION 2000
#define TIME_PER_VEHICLE 800
#define NAME_MAX 16

const char* basedir = "C:\\TrafficShared\\";
const char* files[4] = {
    "C:\\TrafficShared\\lanea.txt",
    "C:\\TrafficShared\\laneb.txt",
    "C:\\TrafficShared\\lanec.txt",
    "C:\\TrafficShared\\laned.txt"
};

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

typedef struct {
    Lane L1;
    Lane L2;
    Lane L3;
} RoadData;

// Road data structure
RoadData roads[4];
TransitionVehicle transitions[MAX_QUEUE];  
int transitionCount = 0;                   
int currentGreen = 0;
int lightState = GREEN_LIGHT;

// Red phase variables
Uint32 lightTimer = 0;
Uint32 dynamicGreenTime = 0;
Uint32 redPhaseStartTime = 0;
int vehiclesToServe = 0;
int vehiclesServed = 0;

// Function to get opposite road
static int getOppositeRoad(int road) {
    switch (road) {
    case 0: return 2;
    case 1: return 3;
    case 2: return 0;
    case 3: return 1;
    }
    return 0;
}

// Queue helper functions
void initLane(Lane* l) {
    l->front = 0;
    l->rear = -1;
    l->count = 0;
}

int enqueue(Lane* l, Vehicle v) {
    if (l->count >= MAX_QUEUE) return 0;
    l->rear = (l->rear + 1) % MAX_QUEUE;
    l->data[l->rear] = v;
    l->count++;
    return 1;
}

static int dequeue(Lane* l, Vehicle* out) {
    if (l->count == 0) return 0;
    *out = l->data[l->front];
    l->front = (l->front + 1) % MAX_QUEUE;
    l->count--;
    return 1;
}

static Vehicle* getLaneVehicle(Lane* l, int index) {
    if (index < 0 || index >= l->count) return NULL;
    int idx = (l->front + index) % MAX_QUEUE;
    return &l->data[idx];
}

// Add distance calculation
float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// Geometry helpers
static int lane_index_for(int road, int logicalLane) {
    if (road == 0) {
        if (logicalLane == 3) return 0;
        if (logicalLane == 2) return 1;
        return 2;
    }
    else if (road == 1) {
        if (logicalLane == 3) return 0;
        if (logicalLane == 2) return 1;
        return 2;
    }
    else if (road == 2) {
        if (logicalLane == 3) return 2;
        if (logicalLane == 2) return 1;
        return 0;
    }
    else {
        if (logicalLane == 3) return 2;
        if (logicalLane == 2) return 1;
        return 0;
    }
}

// Helper function for physical lane coordinates
static void spawn_coords_for_fixed(int road, int laneIndex, float* outx, float* outy) {
    float cx = SCREEN_W / 2.0f;
    float cy = SCREEN_H / 2.0f;
    float startx = cx - ROAD_W / 2.0f;
    float starty = cy - ROAD_W / 2.0f;

    if (road == 0) { *outx = startx + LANE_W * (laneIndex + 0.5f); *outy = -30.0f; }
    else if (road == 1) { *outx = SCREEN_W + 30.0f; *outy = starty + LANE_W * (laneIndex + 0.5f); }
    else if (road == 2) { *outx = startx + LANE_W * (laneIndex + 0.5f); *outy = SCREEN_H + 30.0f; }
    else { *outx = -30.0f; *outy = starty + LANE_W * (laneIndex + 0.5f); }
}

static void intersection_lane_center(int road, int logicalLane, float* outx, float* outy) {
    float cx = SCREEN_W / 2.0f;
    float cy = SCREEN_H / 2.0f;
    float startx = cx - ROAD_W / 2.0f;
    float starty = cy - ROAD_W / 2.0f;

    int idx = lane_index_for(road, logicalLane);
    if (road == 0) { *outx = startx + LANE_W * (idx + 0.5f); *outy = cy - ROAD_W / 2.0f - 8.0f; }
    else if (road == 1) { *outx = cx + ROAD_W / 2.0f + 8.0f; *outy = starty + LANE_W * (idx + 0.5f); }
    else if (road == 2) { *outx = startx + LANE_W * (idx + 0.5f); *outy = cy + ROAD_W / 2.0f + 8.0f; }
    else { *outx = cx - ROAD_W / 2.0f - 8.0f; *outy = starty + LANE_W * (idx + 0.5f); }
}

static int checkTooCloseInLane(Lane* l, float x, float y, int skipIndex) {
    for (int i = 0; i < l->count; i++) {
        if (i == skipIndex) continue;
        Vehicle* other = getLaneVehicle(l, i);
        if (!other) continue;
        float dist = distance(x, y, other->x, other->y);
        if (dist < MIN_SPACING) return 1;
    }
    return 0;
}

static float getDistanceToVehicleAhead(Lane* L, int road, int vehicleIndex) {
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

static void l3_move_vector(int road, float* dx, float* dy) {
    if (road == 0) { *dx = 0.0f; *dy = -VEHICLE_SPEED; }      
    else if (road == 1) { *dx = VEHICLE_SPEED; *dy = 0.0f; }     
    else if (road == 2) { *dx = 0.0f; *dy = VEHICLE_SPEED; }  
    else { *dx = -VEHICLE_SPEED; *dy = 0.0f; }               
}

static void moveLaneL3(Lane* L, int road) {
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
static void addTransition(Vehicle v, int targetRoad) {
    if (transitionCount >= MAX_QUEUE) return;
    v.isStopped = 0;
    transitions[transitionCount].v = v;
    transitions[transitionCount].targetRoad = targetRoad;
    transitions[transitionCount].waitingTime = 0;
    transitionCount++;
}

// movement in transition
static void moveTransitions() {
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

static void cleanupStuckTransitions() {
    static Uint32 lastCleanup = 0;
    Uint32 now = SDL_GetTicks();

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

static void calculateVehiclesToServe() {
    int totalWaiting = roads[currentGreen].L1.count + roads[currentGreen].L2.count;

    vehiclesToServe = totalWaiting;
    if (vehiclesToServe < 1) vehiclesToServe = 1;

    dynamicGreenTime = vehiclesToServe * TIME_PER_VEHICLE;

    printf("Road %d: Calculated |V|=%d vehicles to serve, green time=%dms\n",
        currentGreen, vehiclesToServe, dynamicGreenTime);
}

// Drawing functions
void drawRoads(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_Rect vert = { (int)(SCREEN_W / 2.0f - ROAD_W / 2.0f), 0, ROAD_W, SCREEN_H };
    SDL_Rect horiz = { 0, (int)(SCREEN_H / 2.0f - ROAD_W / 2.0f), SCREEN_W, ROAD_W };
    SDL_RenderFillRect(renderer, &vert);
    SDL_RenderFillRect(renderer, &horiz);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 1; i < 3; i++) {
        int x = (int)(SCREEN_W / 2.0f - ROAD_W / 2.0f + i * LANE_W);
        SDL_RenderDrawLine(renderer, x, 0, x, SCREEN_H);
        int y = (int)(SCREEN_H / 2.0f - ROAD_W / 2.0f + i * LANE_W);
        SDL_RenderDrawLine(renderer, 0, y, SCREEN_W, y);
    }
}

void drawLane(SDL_Renderer* renderer, Lane* L, int r, int g, int b) {
    for (int i = 0; i < L->count; i++) {
        Vehicle* v = getLaneVehicle(L, i);
        if (v) {
            if (v->isStopped) {
                SDL_SetRenderDrawColor(renderer, r / 2, g / 2, b / 2, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            }
            SDL_Rect car = {
                (int)(v->x - VEHICLE_SIZE / 2),
                (int)(v->y - VEHICLE_SIZE / 2),
                VEHICLE_SIZE,
                VEHICLE_SIZE
            };
            SDL_RenderFillRect(renderer, &car);
        }
    }
}

// Draw transitions
void drawTransitions(SDL_Renderer* renderer) {
    for (int i = 0; i < transitionCount; i++) {
        if (transitions[i].v.isStopped) {
            SDL_SetRenderDrawColor(renderer, 128, 90, 0, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 180, 0, 255);
        }
        SDL_Rect car = {
            (int)(transitions[i].v.x - VEHICLE_SIZE / 2),
            (int)(transitions[i].v.y - VEHICLE_SIZE / 2),
            VEHICLE_SIZE,
            VEHICLE_SIZE
        };
        SDL_RenderFillRect(renderer, &car);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
        SDL_RenderDrawRect(renderer, &car);
    }
}

// Add traffic light drawing
void drawTrafficLights(SDL_Renderer* renderer, int currentGreen, int lightState) {
    int coords[4][2] = {
        { (int)(SCREEN_W / 2 - ROAD_W / 2 - 28), (int)(SCREEN_H / 2 - ROAD_W / 2 - 28) },
        { (int)(SCREEN_W / 2 + ROAD_W / 2 + 10), (int)(SCREEN_H / 2 - ROAD_W / 2 - 28) },
        { (int)(SCREEN_W / 2 + ROAD_W / 2 + 10), (int)(SCREEN_H / 2 + ROAD_W / 2 + 10) },
        { (int)(SCREEN_W / 2 - ROAD_W / 2 - 28), (int)(SCREEN_H / 2 + ROAD_W / 2 + 10) }
    };

    if (lightState == RED_PHASE) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < 4; i++) {
            SDL_Rect r = { coords[i][0], coords[i][1], 18, 18 };
            SDL_RenderFillRect(renderer, &r);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        for (int i = 0; i < 4; i++) {
            SDL_Rect border = { coords[i][0] - 2, coords[i][1] - 2, 22, 22 };
            SDL_RenderDrawRect(renderer, &border);
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (i == currentGreen) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            SDL_Rect r = { coords[i][0], coords[i][1], 18, 18 };
            SDL_RenderFillRect(renderer, &r);
        }
    }
}

// File reading
void readVehiclesFromFiles() {
    static int lastReadLines[4] = { 0, 0, 0, 0 };

    for (int roadIdx = 0; roadIdx < 4; roadIdx++) {
        FILE* f = fopen(files[roadIdx], "r");
        if (!f) continue;

        char line[128];
        int currentLine = 0;

        while (currentLine < lastReadLines[roadIdx] && fgets(line, sizeof(line), f)) {
            currentLine++;
        }

        while (fgets(line, sizeof(line), f)) {
            int id, lane;
            char name[NAME_MAX];

            if (sscanf(line, "%d %15s %d", &id, name, &lane) == 3) {
                Vehicle v;
                v.id = id;
                v.fromRoad = roadIdx;
                v.isStopped = 0;
                strncpy(v.name, name, NAME_MAX - 1);
                v.name[NAME_MAX - 1] = '\0';

                Lane* targetLane = NULL;
                int laneIndex = 0;

                if (lane == 1) {
                    targetLane = &roads[roadIdx].L1;
                    laneIndex = lane_index_for(roadIdx, 1);
                }
                else if (lane == 2) {
                    targetLane = &roads[roadIdx].L2;
                    laneIndex = lane_index_for(roadIdx, 2);
                }
                else if (lane == 3) {
                    targetLane = &roads[roadIdx].L3;
                    laneIndex = lane_index_for(roadIdx, 3);
                }

                if (targetLane) {
                    float sx, sy;
                    spawn_coords_for_fixed(roadIdx, laneIndex, &sx, &sy);
                    v.x = sx;
                    v.y = sy;

                    if (!checkTooCloseInLane(targetLane, sx, sy, -1)) {
                        enqueue(targetLane, v);
                    }
                }
            }

            lastReadLines[roadIdx]++;
        }

        fclose(f);
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    if (TTF_Init() != 0) {
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Traffic Simulator - Full Transition Logic",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 16);
    srand((unsigned)time(NULL));

    // Initialize lanes
    for (int i = 0; i < 4; i++) {
        initLane(&roads[i].L1);
        initLane(&roads[i].L2);
        initLane(&roads[i].L3);
    }

    readVehiclesFromFiles();

    // Initialize transition state 
    calculateVehiclesToServe();
    lightTimer = SDL_GetTicks();

    int running = 1;
    SDL_Event e;
    Uint32 lastFileCheck = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        Uint32 now = SDL_GetTicks();

        // Check for new vehicles
        if (now - lastFileCheck >= 200) {
            readVehiclesFromFiles();
            lastFileCheck = now;
        }

        cleanupStuckTransitions();

        // Move vehicles
        for (int r = 0; r < 4; r++) {
            moveLaneL3(&roads[r].L3, r);
        }

        moveTransitions();

        Uint32 elapsed = now - lightTimer;

        if (lightState == GREEN_LIGHT) {
            if (elapsed >= dynamicGreenTime || vehiclesServed >= vehiclesToServe) {
                printf("\n--- Ending Green Phase for Road %d ---\n", currentGreen);
                printf("Starting RED_PHASE (all lights RED for clearance)\n");
                printf("Transition vehicles to clear: %d\n", transitionCount);

                lightState = RED_PHASE;
                redPhaseStartTime = now;
                lightTimer = now;

                for (int r = 0; r < 4; r++) {
                    for (int i = 0; i < roads[r].L1.count; i++) {
                        Vehicle* v = getLaneVehicle(&roads[r].L1, i);
                        if (v) v->isStopped = 1;
                    }
                    for (int i = 0; i < roads[r].L2.count; i++) {
                        Vehicle* v = getLaneVehicle(&roads[r].L2, i);
                        if (v) v->isStopped = 1;
                    }
                }
            }
            else {
                moveLaneTowardCenter(&roads[currentGreen].L1, currentGreen);
                moveLaneTowardCenter(&roads[currentGreen].L2, currentGreen);

                for (int li = 0; li < 2; li++) {
                    Lane* L = (li == 0) ? &roads[currentGreen].L1 : &roads[currentGreen].L2;
                    int cnt = L->count;

                    for (int i = 0; i < cnt; i++) {
                        Vehicle* v = getLaneVehicle(L, i);
                        if (!v) continue;

                        int reached = 0;
                        float cx = SCREEN_W / 2.0f, cy = SCREEN_H / 2.0f;
                        if (currentGreen == 0 && v->y >= cy - ROAD_W / 2.0f) reached = 1;
                        if (currentGreen == 1 && v->x <= cx + ROAD_W / 2.0f) reached = 1;
                        if (currentGreen == 2 && v->y <= cy + ROAD_W / 2.0f) reached = 1;
                        if (currentGreen == 3 && v->x >= cx - ROAD_W / 2.0f) reached = 1;

                        if (reached && vehiclesServed < vehiclesToServe) {
                            Vehicle temp;
                            dequeue(L, &temp);
                            vehiclesServed++;

                            int targetRoad;
                            if (L == &roads[currentGreen].L1) {
                                targetRoad = (currentGreen + 1) % 4;
                            }
                            else {
                                int opposite = getOppositeRoad(currentGreen);
                                if (rand() % 2 == 0) {
                                    targetRoad = opposite;
                                }
                                else {
                                    targetRoad = (currentGreen + 3) % 4;
                                }
                            }
                            addTransition(temp, targetRoad);
                            i--; cnt--;
                        }
                    }
                }
            }
        }
        else if (lightState == RED_PHASE) {
            Uint32 redElapsed = now - redPhaseStartTime;

            if (redElapsed >= RED_PHASE_DURATION || transitionCount == 0) {
                printf("\n--- Ending Clearance Phase ---\n");
                printf("Remaining transition vehicles: %d\n", transitionCount);

                currentGreen = (currentGreen + 1) % 4;

                printf("Next green light: Road %d\n", currentGreen);

                lightState = GREEN_LIGHT;
                vehiclesServed = 0;
                calculateVehiclesToServe();
                lightTimer = now;

                for (int r = 0; r < 4; r++) {
                    for (int i = 0; i < roads[r].L1.count; i++) {
                        Vehicle* v = getLaneVehicle(&roads[r].L1, i);
                        if (v) v->isStopped = 0;
                    }
                    for (int i = 0; i < roads[r].L2.count; i++) {
                        Vehicle* v = getLaneVehicle(&roads[r].L2, i);
                        if (v) v->isStopped = 0;
                    }
                }
            }
        }

        for (int r = 0; r < 4; r++) {
            if (r != currentGreen || lightState == RED_PHASE) {
                moveLaneTowardCenter(&roads[r].L1, r);
                moveLaneTowardCenter(&roads[r].L2, r);
            }
        }

        // Draw everything
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawRoads(renderer);

        for (int r = 0; r < 4; r++) {
            drawLane(renderer, &roads[r].L1, 220, 80, 80);
            drawLane(renderer, &roads[r].L2, 80, 220, 80);
            drawLane(renderer, &roads[r].L3, 80, 120, 220);
        }

        // Draw transitions
        drawTransitions(renderer);

        drawTrafficLights(renderer, currentGreen, lightState);

        SDL_RenderPresent(renderer);
        sleep_ms(16);
    }

    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

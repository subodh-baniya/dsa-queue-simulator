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
#define RED_LIGHT 1

const char* basedir = "C:\\TrafficShared\\";
const char* files[4] = {
    "C:\\TrafficShared\\lanea.txt",
    "C:\\TrafficShared\\laneb.txt",
    "C:\\TrafficShared\\lanec.txt",
    "C:\\TrafficShared\\laned.txt"
};


//Vehicle structure
typedef struct {
    int id;
    float x, y;
    int fromRoad;
    int isStopped;
} Vehicle;

//Circular queue for lanes
typedef struct {
    Vehicle data[MAX_QUEUE];
    int front, rear, count;
} Lane;

typedef struct {
    Lane L1;
    Lane L2;
    Lane L3;
} RoadData;

//Road data structure
RoadData roads[4];
int currentGreen = 0; 
int lightState = GREEN_LIGHT;

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

void spawn_coords_for(int road, int logicalLane, float* outx, float* outy) {
    int laneIndex = lane_index_for(road, logicalLane);
    spawn_coords_for_fixed(road, laneIndex, outx, outy);
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


static void moveLaneL3(Lane* L, int road) {
    float dx = 0.0f, dy = 0.0f;

    if (road == 0) { dy = -VEHICLE_SPEED; }      
    else if (road == 1) { dx = VEHICLE_SPEED; }  
    else if (road == 2) { dy = VEHICLE_SPEED; }  
    else { dx = -VEHICLE_SPEED; }               

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

        if (!checkTooCloseInLane(L, newx, newy, i)) {
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

        if ((lightState == RED_LIGHT || currentGreen != road) &&
            distToIntersection < STOPPING_DISTANCE && distToIntersection > 0) {
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

// Add traffic light drawing
void drawTrafficLights(SDL_Renderer* renderer) {
    int coords[4][2] = {
        { (int)(SCREEN_W / 2 - ROAD_W / 2 - 28), (int)(SCREEN_H / 2 - ROAD_W / 2 - 28) },
        { (int)(SCREEN_W / 2 + ROAD_W / 2 + 10), (int)(SCREEN_H / 2 - ROAD_W / 2 - 28) },
        { (int)(SCREEN_W / 2 + ROAD_W / 2 + 10), (int)(SCREEN_H / 2 + ROAD_W / 2 + 10) },
        { (int)(SCREEN_W / 2 - ROAD_W / 2 - 28), (int)(SCREEN_H / 2 + ROAD_W / 2 + 10) }
    };

    for (int i = 0; i < 4; i++) {
        if (i == currentGreen && lightState == GREEN_LIGHT) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        SDL_Rect r = { coords[i][0], coords[i][1], 18, 18 };
        SDL_RenderFillRect(renderer, &r);
    }
}

// File reading
void readVehiclesFromFiles() {
    for (int roadIdx = 0; roadIdx < 4; roadIdx++) {
        FILE* f = fopen(files[roadIdx], "r");
        if (!f) continue;

        char line[128];
        while (fgets(line, sizeof(line), f)) {
            int id, lane;
            char name[16];

            if (sscanf(line, "%d %15s %d", &id, name, &lane) == 3) {
                Vehicle v;
                v.id = id;
                v.fromRoad = roadIdx;
                v.isStopped = 0;

                float sx, sy;
                spawn_coords_for(roadIdx, lane, &sx, &sy);
                v.x = sx;
                v.y = sy;

                if (lane == 1) enqueue(&roads[roadIdx].L1, v);
                else if (lane == 2) enqueue(&roads[roadIdx].L2, v);
                else if (lane == 3) enqueue(&roads[roadIdx].L3, v);
            }
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

    SDL_Window* window = SDL_CreateWindow("Traffic Simulator - With Traffic Lights",
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

    int running = 1;
    SDL_Event e;
    Uint32 lastFileCheck = 0;
    Uint32 lastLightChange = 0;

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

        // Change traffic lights every 5 seconds
        if (now - lastLightChange >= 5000) {
            currentGreen = (currentGreen + 1) % 4;
            lastLightChange = now;
        }

        // Move vehicles
        for (int r = 0; r < 4; r++) {
            moveLaneTowardCenter(&roads[r].L1, r);
            moveLaneTowardCenter(&roads[r].L2, r);
            moveLaneL3(&roads[r].L3, r);
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

        drawTrafficLights(renderer);

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

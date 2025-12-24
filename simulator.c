
#define _CRT_SECURE_NO_WARNINGS 
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <string.h>

#include "constants.h"
#include "vehicle.h"
#include "geometry.h"
#include "movement.h"
#include "file_reader.h"
#include "renderer.h"
#include "globals.h"

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

#define _CRT_SECURE_NO_WARNINGS 
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <string.h>

#include "./headers/constants.h"
#include "./headers/vehicle.h"
#include "./headers/geometry.h"
#include "./headers/movement.h"
#include "./headers/file_reader.h"
#include "./headers/renderer.h"
#include "./headers/globals.h"

//render text using sdl_ttf
static void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

// Status info drawing function
static void drawStatusInfo(SDL_Renderer* renderer, TTF_Font* font, Uint32 timeRemaining) {
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 255, 0, 255 };
    SDL_Color green = { 0, 255, 0, 255 };
    SDL_Color red = { 255, 100, 100, 255 };

    char buffer[256];
    const char* roadNames[] = { "Road A (North)", "Road B (East)", "Road C (South)", "Road D (West)" };

    sprintf(buffer, "Current State: %s", lightState == RED_PHASE ? "CLEARANCE PHASE (All Red)" : "NORMAL");
    drawText(renderer, font, buffer, 10, 70, lightState == RED_PHASE ? yellow : white);

    if (lightState == GREEN_LIGHT) {
        sprintf(buffer, "Green: %s", roadNames[currentGreen]);
        drawText(renderer, font, buffer, 10, 95, green);

        sprintf(buffer, "Time: %.1fs", timeRemaining / 1000.0f);
        drawText(renderer, font, buffer, 10, 120, white);
    }
    else {
        Uint32 redTimeElapsed = SDL_GetTicks() - redPhaseStartTime;
        Uint32 redTimeLeft = RED_PHASE_DURATION - redTimeElapsed;
        if (redTimeLeft > RED_PHASE_DURATION) redTimeLeft = 0;

        sprintf(buffer, "Clearance Phase: %.1fs remaining", redTimeLeft / 1000.0f);
        drawText(renderer, font, buffer, 10, 95, yellow);
    }

    sprintf(buffer, "Transitions: %d vehicles", transitionCount);
    drawText(renderer, font, buffer, 10, 145, white);

    drawText(renderer, font, "=== PRIORITY QUEUE STATUS ===", 10, 170, yellow);

    for (int i = 0; i < 4; i++) {
        int priority = NORMAL_PRIORITY;
        for (int j = 0; j < trafficQueue.size; j++) {
            if (trafficQueue.elements[j].roadIndex == i) {
                priority = trafficQueue.elements[j].priority;
                break;
            }
        }

        sprintf(buffer, "%s: L1=%d L2=%d [%s]",
            roadNames[i],
            roads[i].L1.count,
            roads[i].L2.count,
            priority == HIGH_PRIORITY ? "HIGH" : "Normal");

        SDL_Color color = white;
        if (priority == HIGH_PRIORITY) {
            color = red;
        }
        else if (roads[i].L2.count > PRIORITY_THRESHOLD / 2) {
            color = yellow;
        }

        drawText(renderer, font, buffer, 10, 195 + i * 25, color);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("TTF init error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Traffic Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 16);
    if (!font) {
        printf("Warning: Could not load font. Text will not display.\n");
    }

    srand((unsigned)time(NULL));

    // Initialize lanes
    for (int i = 0; i < 4; i++) {
        initLaneWithMax(&roads[i].L1, 0);
        initLaneWithMax(&roads[i].L2, 50);
        initLaneWithMax(&roads[i].L3, 0);
    }

    initPriorityQueue(&trafficQueue);

    int running = 1;
    SDL_Event e;
    Uint32 lastFileCheck = 0;
    Uint32 lastPriorityUpdate = 0;

	// Initialize transition state
    calculateVehiclesToServe();
    lightTimer = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        Uint32 now = SDL_GetTicks();

        if (now - lastFileCheck >= FILE_CHECK_INTERVAL) {
            readVehiclesFromFiles();
            lastFileCheck = now;
        }

        if (now - lastPriorityUpdate >= 1000) {
            updateAllPriorities(&trafficQueue);

            static int printCounter = 0;
            printCounter++;
            if (printCounter >= 5) {
                for (int i = 0; i < 4; i++) {
                    int priority = NORMAL_PRIORITY;
                    for (int j = 0; j < trafficQueue.size; j++) {
                        if (trafficQueue.elements[j].roadIndex == i) {
                            priority = trafficQueue.elements[j].priority;
                            break;
                        }
                    }
                }
                printCounter = 0;
            }

            lastPriorityUpdate = now;
        }

        cleanupStuckTransitions();

        for (int r = 0; r < 4; r++) {
            moveLaneL3(&roads[r].L3, r);
        }

        moveTransitions();

        Uint32 elapsed = now - lightTimer;

        if (lightState == GREEN_LIGHT) {
            if (elapsed >= dynamicGreenTime || vehiclesServed >= vehiclesToServe) {

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

                currentGreen = getNextRoadToServe(&trafficQueue, now);

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

        Uint32 timeRemaining;
        if (lightState == GREEN_LIGHT) {
            timeRemaining = dynamicGreenTime - (now - lightTimer);
            if (timeRemaining > dynamicGreenTime) timeRemaining = 0;
        }
        else {
            timeRemaining = RED_PHASE_DURATION - (now - redPhaseStartTime);
            if (timeRemaining > RED_PHASE_DURATION) timeRemaining = 0;
        }
        drawStatusInfo(renderer, font, timeRemaining);

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
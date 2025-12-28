#include "./headers/renderer.h"
#include "./headers/globals.h"

// Draw roads and lane markings
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

// Draw all vehicles in a lane with specified color
void drawLane(SDL_Renderer* renderer, Lane* L, int r, int g, int b) {
    for (int i = 0; i < L->count; i++) {
        Vehicle* v = getLaneVehicle(L, i);
        if (v) {
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_Rect car = {
                (int)(v->x - VEHICLE_SIZE / 2),
                (int)(v->y - VEHICLE_SIZE / 2),
                VEHICLE_SIZE,
                VEHICLE_SIZE
            };
            SDL_RenderFillRect(renderer, &car);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
            SDL_RenderDrawRect(renderer, &car);
        }
    }
}

// Draw vehicles that are transitioning between roads
void drawTransitions(SDL_Renderer* renderer) {
    for (int i = 0; i < transitionCount; i++) {
            SDL_SetRenderDrawColor(renderer, 255, 180, 0, 255);
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

// Draw traffic light indicators at intersection corners
// Shows which road has green light, or all red during clearance phase
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
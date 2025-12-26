
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "vehicle.h"
#include "geometry.h"

void drawRoads(SDL_Renderer* renderer); // Draws roads and lane markings
void drawLane(SDL_Renderer* renderer, Lane* L, int r, int g, int b);// Draws all vehicles in a lane with specified color
void drawTransitions(SDL_Renderer* renderer); // Draws vehicles that are transitioning between roads
void drawTrafficLights(SDL_Renderer* renderer, int currentGreen, int lightState); // Draws traffic light indicators at intersection corners

#endif
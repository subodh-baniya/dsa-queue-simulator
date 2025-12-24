
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "vehicle.h"
#include "geometry.h"

void drawRoads(SDL_Renderer* renderer);
void drawLane(SDL_Renderer* renderer, Lane* L, int r, int g, int b);
void drawTransitions(SDL_Renderer* renderer);
void drawTrafficLights(SDL_Renderer* renderer, int currentGreen, int lightState);

#endif
#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <math.h>
#include <string.h>

// Screen dimensions
#define SCREEN_W 900
#define SCREEN_H 900
#define ROAD_W 200
#define LANE_W (ROAD_W/3.0f)

// Simulation parameters
#define MAX_QUEUE 50
#define VEHICLE_SPEED 2.0f
#define VEHICLE_SIZE 12
#define STOPPING_DISTANCE 30.0f
#define MIN_SPACING 20.0f
#define MIN_FRONT_SPACING 25.0f

// Traffic light states
#define GREEN_LIGHT 0
#define RED_PHASE 1
#define RED_PHASE_DURATION 2000
#define TIME_PER_VEHICLE 800

// Other constants
#define NAME_MAX 16
#define FILE_CHECK_INTERVAL 200
#define sleep_ms(x) Sleep(x)

// File paths
extern const char* files[4];

// Global simulation state
typedef struct {
    int currentGreen;
    int lightState;
    Uint32 lightTimer;
    Uint32 dynamicGreenTime;
    Uint32 redPhaseStartTime;
    int vehiclesToServe;
    int vehiclesServed;
    Uint32 lastFileCheck;
} SimulationState;

#endif
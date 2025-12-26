// Defining all constants
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SCREEN_W 900
#define SCREEN_H 900
#define ROAD_W 200
#define LANE_W (ROAD_W/3.0f)
#define MAX_QUEUE 120
#define TIME_PER_VEHICLE 800
#define FILE_CHECK_INTERVAL 200
#define VEHICLE_SPEED 2.5f
#define VEHICLE_SIZE 12
#define MIN_SPACING 20.0f
#define MIN_FRONT_SPACING 25.0f
#define STOPPING_DISTANCE 30.0f
#define sleep_ms(x) Sleep(x)
#define NAME_MAX 16

#define GREEN_LIGHT 0
#define RED_PHASE 1
#define RED_PHASE_DURATION 2000

#define PRIORITY_THRESHOLD 10    
#define PRIORITY_RESET 5          
#define NORMAL_PRIORITY 1
#define HIGH_PRIORITY 100

#endif
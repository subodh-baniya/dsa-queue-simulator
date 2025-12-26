// Defining all constants
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SCREEN_W 900 // Window width
#define SCREEN_H 900 // Window height
#define ROAD_W 200   // Road width 
#define LANE_W (ROAD_W/3.0f) // Each lane is 1/3 of road width
#define MAX_QUEUE 120  // Maximum vehicles in queue 

#define TIME_PER_VEHICLE 800    // 0.8 seconds per vehicle
#define FILE_CHECK_INTERVAL 200  // Check files every 0.2 seconds
#define VEHICLE_SPEED 2.5f   // How fast cars move
#define VEHICLE_SIZE 12    // Car size on screen
#define MIN_SPACING 20.0f    // Cars shouldn't be too close
#define MIN_FRONT_SPACING 25.0f  // Extra space in front
#define STOPPING_DISTANCE 30.0f  // When to stop at lights

#define sleep_ms(x) Sleep(x)
#define NAME_MAX 16                // Max vehicle name length

#define GREEN_LIGHT 0 // Green light phase
#define RED_PHASE 1 // Red light phase
#define RED_PHASE_DURATION 2000    // 2 second clearance phase

#define PRIORITY_THRESHOLD 10    // Becomes HIGH when L2 > 10
#define PRIORITY_RESET 5    // Back to NORMAL when L2 < 5
#define NORMAL_PRIORITY 1
#define HIGH_PRIORITY 100   

#endif
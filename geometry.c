
#include "geometry.h"
#include "vehicle.h"
#include <math.h>
#include "globals.h"


int lane_index_for(int road, int logicalLane) {
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

void spawn_coords_for_fixed(int road, int laneIndex, float* outx, float* outy) {
    float cx = SCREEN_W / 2.0f;
    float cy = SCREEN_H / 2.0f;
    float startx = cx - ROAD_W / 2.0f;
    float starty = cy - ROAD_W / 2.0f;

    if (road == 0) { *outx = startx + LANE_W * (laneIndex + 0.5f); *outy = -30.0f; }
    else if (road == 1) { *outx = SCREEN_W + 30.0f; *outy = starty + LANE_W * (laneIndex + 0.5f); }
    else if (road == 2) { *outx = startx + LANE_W * (laneIndex + 0.5f); *outy = SCREEN_H + 30.0f; }
    else { *outx = -30.0f; *outy = starty + LANE_W * (laneIndex + 0.5f); }
}

void intersection_lane_center(int road, int logicalLane, float* outx, float* outy) {
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

void l3_move_vector(int road, float* dx, float* dy) {
    if (road == 0) { *dx = 0.0f; *dy = -VEHICLE_SPEED; }
    else if (road == 1) { *dx = VEHICLE_SPEED; *dy = 0.0f; }
    else if (road == 2) { *dx = 0.0f; *dy = VEHICLE_SPEED; }
    else { *dx = -VEHICLE_SPEED; *dy = 0.0f; }
}
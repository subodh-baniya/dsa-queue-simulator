
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "constants.h"

int lane_index_for(int road, int logicalLane);
void spawn_coords_for_fixed(int road, int laneIndex, float* outx, float* outy);
void intersection_lane_center(int road, int logicalLane, float* outx, float* outy);
void l3_move_vector(int road, float* dx, float* dy);

#endif

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "constants.h"

int lane_index_for(int road, int logicalLane);  // Converts a logical lane number to a physical screen index based on which road we're on.
void spawn_coords_for_fixed(int road, int laneIndex, float* outx, float* outy); // Calculates where new vehicles should appear on screen when they first enter the simulation.
void intersection_lane_center(int road, int logicalLane, float* outx, float* outy); // Finds the exact center point of a lane at the intersection where vehicles stop when waiting for green light,and where transitioning vehicles aim for when changing roads.
void l3_move_vector(int road, float* dx, float* dy); // Provides the movement vector for vehicles in lane 3, which have a unique turning behavior at the intersection.

#endif
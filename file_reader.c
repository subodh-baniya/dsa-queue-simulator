#define _CRT_SECURE_NO_WARNINGS
#include "file_reader.h"
#include <stdio.h>
#include <string.h>
#include "globals.h"

// Reads vehicle data from input files and spawns them into simulation.
void readVehiclesFromFiles() {
    static int lastReadLines[4] = { 0, 0, 0, 0 };

    for (int roadIdx = 0; roadIdx < 4; roadIdx++) {
        FILE* f = fopen(files[roadIdx], "r");
        if (!f) continue;

        char line[128];
        int currentLine = 0;

        while (currentLine < lastReadLines[roadIdx] && fgets(line, sizeof(line), f)) {
            currentLine++;
        }

        while (fgets(line, sizeof(line), f)) {
            int id, lane;
            char name[NAME_MAX];

            if (sscanf(line, "%d %15s %d", &id, name, &lane) == 3) {
                Vehicle v;
                v.id = id;
                v.fromRoad = roadIdx;
                v.isStopped = 0;
                v.isTransitioning = 0;
                strncpy(v.name, name, NAME_MAX - 1);
                v.name[NAME_MAX - 1] = '\0';

                Lane* targetLane = NULL;
                int laneIndex = 0;

                if (lane == 1) {
                    targetLane = &roads[roadIdx].L1;
                    laneIndex = lane_index_for(roadIdx, 1);
                }
                else if (lane == 2) {
                    targetLane = &roads[roadIdx].L2;
                    laneIndex = lane_index_for(roadIdx, 2);
                }
                else if (lane == 3) {
                    targetLane = &roads[roadIdx].L3;
                    laneIndex = lane_index_for(roadIdx, 3);
                }

                if (targetLane) {
                    float sx, sy;
                    spawn_coords_for_fixed(roadIdx, laneIndex, &sx, &sy);
                    v.x = sx;
                    v.y = sy;

                    if (!checkTooCloseInLane(targetLane, sx, sy, -1)) {
                        enqueue(targetLane, v);
                    }
                }
            }

            lastReadLines[roadIdx]++;
        }

        fclose(f);
    }
}
#define _CRT_SECURE_NO_WARNINGS //make sure old functions also run
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <direct.h>

const char* baseDir = "C:\\TrafficShared"; // folder to store the generate data files
const char* files[4] = {       //file path for each road
    "C:\\TrafficShared\\lanea.txt",
    "C:\\TrafficShared\\laneb.txt",
    "C:\\TrafficShared\\lanec.txt",
    "C:\\TrafficShared\\laned.txt"
};

#define DEFAULT_INTERVAL_MS 300 // delay between vehicle generations
#define NAME_MAX 16  // max vehicle name length
#define MIN_SPAWN_SPACING_MS 300 // minimum delay between generation on the same lane


//stores the last time a vehicle spawned on each lane
typedef struct {
    DWORD lastSpawnTime[3];
} RoadSpawnTracker;

RoadSpawnTracker spawnTrackers[4] = { 0 };//array for 4 roads


// check if this road+lane waited long enough
static int canSpawnOnLane(int road, int lane) {
    DWORD now = GetTickCount();
    DWORD last = spawnTrackers[road].lastSpawnTime[lane - 1];
    if (now - last >= MIN_SPAWN_SPACING_MS) {
        spawnTrackers[road].lastSpawnTime[lane - 1] = now;
        return 1;
    }
    return 0;
}


// write a vehicle line into the selected road file
static int append_vehicle_to_file(int road, int id, const char* name, int lane) {
    FILE* f = fopen(files[road], "a");
    if (!f) return 0;
    fprintf(f, "%d %s %d\n", id, name, lane);
    fclose(f);
    return 1;
}


// pick lane using probability
static int choose_lane_weighted() {
    int r = rand() % 100;
	if (r < 25) return 1; //25% probability for lane 1
	if (r < 85) return 2; //60% probability for lane 2
    return 3;
}


// lane selection with spacing check
static int choose_lane_safe(int road) {
    int p = choose_lane_weighted();
    if (canSpawnOnLane(road, p)) return p;
    for (int i = 1; i <= 3; i++) {
        if (canSpawnOnLane(road, i)) return i;
    }
    return -1;
}


// clear all data files at start
static void clear_all_files() {
    for (int i = 0; i < 4; i++) {
        FILE* f = fopen(files[i], "w");
        if (f) fclose(f);
    }
}

int main(int argc, char** argv) {
    int interval_ms = DEFAULT_INTERVAL_MS;

    for (int i = 1; i < argc; i++) {
        int t = atoi(argv[i]);
        if (t > 0) interval_ms = t;
    }

    _mkdir(baseDir);   // create folder if missing
    clear_all_files(); //clear old data

    srand((unsigned)time(NULL) ^ (unsigned)GetTickCount());
    int nextId = 1;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            spawnTrackers[i].lastSpawnTime[j] = 0;
        }
    }

    int roadIdx = 0;

    while (1) {
        int road = roadIdx % 4;
        roadIdx++;

        int lane = choose_lane_safe(road);
        if (lane == -1) {
            Sleep(interval_ms);
            continue;
        }

        char name[NAME_MAX];
        snprintf(name, sizeof(name), "veh%d", nextId);

        if (append_vehicle_to_file(road, nextId, name, lane)) {
            printf("Generated ID=%d Lane=%d Road=%d\n", nextId, lane, road + 1);
        }

        nextId++;
        Sleep(interval_ms);
    }

    return 0;
}

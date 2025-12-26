# Traffic Junction Simulator🚦
This is a data structure and algorithms project implementing a queue-based traffic management system for a four-way intersection with priority lanes and free turning lanes.

# 📋 Overview
This project simulates a traffic junction where vehicles navigate through an intersection goverened by traffic lights and lane-specific rules , demostrating the use of queue data structure.
This project implements a complete **traffic simulation system** using **C and SDL2**, composed of two interconnected programs:

1. **Traffic Generator**  
   Generates realistic vehicle traffic data and writes it to shared text files.

2. **Traffic Simulator**  
   Reads generated data and visualizes a real-time traffic flow with smart traffic lights and collision-aware vehicle movement.

# ✨ Key Features
- **Queue-based Traffic Management**: Using linear data structures to solve a real-world problem.
- **Priority-Based Traffic Control**: Smart traffic lights works based on priority.
- **Real-Time Vehicle Generation**: Separate generator program creates realistic traffic patterns.
- **Collision-Free Movement**: Mathematical spacing algorithms prevent vehicle collisions.
- **Dynamic Traffic Light Timing**: Lights duration calculated based on waiting vehicle count.
- **Realistic Vehicle Movement**: Proper queuing and turning animations.

# 📊Data Structure Implementation
| Data Structure | Implementation |Purpose |
|----------------|----------------|--------|
|Vehicle|typedef struct {
    int id;
    float x, y;
    int fromRoad;
    char name[NAME_MAX];
    int isStopped;
    int isTransitioning;
} Vehicle;|Represents individual vehicles in simulation with position, state, and identification data.|
|Lane|typedef struct {
    Vehicle data[MAX_QUEUE];
    int front, rear, count;
    int maxAllowed;
} Lane;|Implements a queue to manage vehicles in each lane, supporting enqueue, dequeue, and state tracking.|
|RoadData|typedef struct {
    Lane L1;
    Lane L2;
    Lane L3;
} RoadData;|Holds three lanes for each road,for organized management.|
|TransitionVehicle|typedef struct {
    Vehicle v;
    int targetRoad;
    int waitingTime;
} TransitionVehicle;|Temporarily holds vehicles in transition between roads, tracking their target and wait time.|
|PriorityQueueElement|typedef struct {
    int roadIndex;
    int priority;
    Uint32 lastServedTime;
} PriorityQueueElement;| Stores priority data for individual road including current priority and last green light time.|
|TrafficPriorityQueue|typedef struct {
    PriorityQueueElement elements[4];
    int size;
} TrafficPriorityQueue;|Manages the priority queue for traffic lights, determining which road gets green light based on vehicle count and wait times.|
|files[4]|const char* files[4] = {
    "C:\\TrafficShared\\lanea.txt",
    "C:\\TrafficShared\\laneb.txt",
    "C:\\TrafficShared\\lanec.txt",
    "C:\\TrafficShared\\laned.txt"
};|Holds file paths for shared text files used for inter-process communication between the traffic generator and simulator.|
|roads[4]|RoadData roads[4]; in globals.c|Contains all 4 roads (North, East, South, West) each with 3 lanes for vehicle management.|
|spawnTrackers[4]|RoadSpawnTracker spawnTrackers[4] = {0}; in traffic_generator.c|Tracks last spawn timestamp for each road to prevent rapid vehicle spawning.|
|transitions[MAX_QUEUE]|TransitionVehicle transitions[MAX_QUEUE]; in globals.c|Stores vehicles currently transitioning between roads during intersection crossing.|
|trafficQueue.elements[4]|PriorityQueueElement elements[4]; inside TrafficPriorityQueue|Stores priority data (value + timestamp) for each of the 4 roads.|
|RoadSpawnTracker.lastSpawnTime[3]|DWORD lastSpawnTime[3]; inside RoadSpawnTracker|Tracks last spawn time for 3 lanes (L1, L2, L3) per road to enforce spacing.|
|coords[4][2]|int coords[4][2] = { {x1,y1}, {x2,y2}, {x3,y3}, {x4,y4} }; in renderer.c|Stores screen coordinates for traffic light indicator positions at intersection corners|
|lastReadLines[4]|static int lastReadLines[4] = { 0, 0, 0, 0 }; in file_reader.c|Tracks how many lines have been read from each data file to avoid re-reading same vehicles.|
|Lane.data[MAX_QUEUE]|Vehicle data[MAX_QUEUE]; inside Lane structure|Circular buffer storing up to 120 vehicles per lane with front/rear pointer management.|






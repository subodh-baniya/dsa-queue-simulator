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

# 📊 Data Structure Implementation

| Data Structure | Implementation | Purpose |
|---------------|----------------|---------|
| **Vehicle** | `typedef struct { int id; float x, y; int fromRoad; char name[NAME_MAX]; int isStopped; int isTransitioning; } Vehicle;` | Represents an individual vehicle with position, state, and identification. |
| **Lane** | `typedef struct { Vehicle data[MAX_QUEUE]; int front, rear, count; int maxAllowed; } Lane;` | Queue structure for managing vehicles per lane with enqueue/dequeue control. |
| **RoadData** | `typedef struct { Lane L1; Lane L2; Lane L3; } RoadData;` | Groups three lanes (L1, L2, L3) for a single road. |
| **TransitionVehicle** | `typedef struct { Vehicle v; int targetRoad; int waitingTime; } TransitionVehicle;` | Holds vehicles while crossing the intersection and switching roads. |
| **PriorityQueueElement** | `typedef struct { int roadIndex; int priority; Uint32 lastServedTime; } PriorityQueueElement;` | Stores priority and last-green timestamp for each road. |
| **TrafficPriorityQueue** | `typedef struct { PriorityQueueElement elements[4]; int size; } TrafficPriorityQueue;` | Controls traffic light decisions using road priorities and wait times. |
| **files[4]** | `const char* files[4] = { "C:\\TrafficShared\\lanea.txt", "C:\\TrafficShared\\laneb.txt", "C:\\TrafficShared\\lanec.txt", "C:\\TrafficShared\\laned.txt" };` | File paths for inter-process communication between generator and simulator. |
| **roads[4]** | `RoadData roads[4];` *(globals.c)* | Stores all four roads (N, E, S, W), each with three lanes. |
| **spawnTrackers[4]** | `RoadSpawnTracker spawnTrackers[4] = {0};` *(traffic_generator.c)* | Prevents excessive vehicle spawning by tracking last spawn time. |
| **transitions[MAX_QUEUE]** | `TransitionVehicle transitions[MAX_QUEUE];` *(globals.c)* | Active list of vehicles currently inside the intersection. |
| **trafficQueue.elements[4]** | `PriorityQueueElement elements[4];` | Per-road priority data used by the traffic light controller. |
| **RoadSpawnTracker.lastSpawnTime[3]** | `DWORD lastSpawnTime[3];` | Enforces spacing between vehicle spawns per lane. |
| **coords[4][2]** | `int coords[4][2] = { {x1,y1}, {x2,y2}, {x3,y3}, {x4,y4} };` *(renderer.c)* | Screen coordinates for traffic light indicators. |
| **lastReadLines[4]** | `static int lastReadLines[4] = {0,0,0,0};` *(file_reader.c)* | Prevents re-reading previously processed vehicle data. |
| **Lane.data[MAX_QUEUE]** | `Vehicle data[MAX_QUEUE];` | Circular buffer storing up to 120 vehicles per lane. |







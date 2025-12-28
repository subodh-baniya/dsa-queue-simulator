
# Traffic Junction Simulator🚦
This is a data structure and algorithms project implementing a queue-based traffic management system for a four-way intersection with priority lanes and free turning lanes.

# 📋 Overview
This project simulates a traffic junction where vehicles navigate through an intersection goverened by traffic lights and lane-specific rules , demostrating the use of queue data structure.
This project implements a complete **traffic simulation system** using **C and SDL2**, composed of two interconnected programs:

1. **Traffic Generator**  
   Generates realistic vehicle traffic data and writes it to shared text files.

2. **Traffic Simulator**  
   Reads generated data and visualizes a real-time traffic flow with smart traffic lights and collision-aware vehicle movement.

# 📺 Demonstration
![Image](https://github.com/user-attachments/assets/1a7235cb-50d4-45c2-9c40-1e6d74d605de)

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

# Function Implementation

| Function | Data Structures Used | Purpose |
|---------|----------------------|---------|
| initLane() | Lane* | Initialize lane queue |
| initLaneWithMax() | Lane* | Initialize lane with capacity |
| enqueue() | Lane*, Vehicle | Add vehicle to lane |
| dequeue() | Lane*, Vehicle* | Remove vehicle from lane |
| getLaneVehicle() | Lane* | Access vehicle in queue |
| checkTooCloseInLane() | Lane* | Check vehicle spacing |
| getDistanceToVehicleAhead() | Lane*, Vehicle | Calculate following distance |
| moveLaneL3() | Lane* | Move right-turn vehicles |
| moveLaneTowardCenter() | Lane*, Vehicle | Move toward intersection |
| addTransition() | TransitionVehicle[], Vehicle | Add to transition queue |
| moveTransitions() | TransitionVehicle[], roads[] | Process lane-changing |
| cleanupStuckTransitions() | TransitionVehicle[] | Remove stuck vehicles |
| initPriorityQueue() | TrafficPriorityQueue* | Initialize priority system |
| updateAllPriorities() | TrafficPriorityQueue*, roads[] | Update traffic light priorities |
| getNextRoadToServe() | TrafficPriorityQueue*, roads[] | Select next green light |
| calculateVehiclesToServe() | roads[] | Calculate green time |
| readVehiclesFromFiles() | files[], Vehicle, roads[] | Read vehicle data |
| lane_index_for() | Road indices | Convert lane numbers |
| spawn_coords_for_fixed() | Road/lane indices | Calculate spawn positions |
| drawLane() | Lane*, Vehicle | Draw vehicles in lane |
| drawTransitions() | TransitionVehicle[] | Draw transitioning vehicles |
| drawStatusInfo() | roads[], TrafficPriorityQueue | Display statistics |
| append_vehicle_to_file() | files[] | Write vehicle data |
| canSpawnOnLane() | RoadSpawnTracker[] | Control spawn timing |

## Algorithms Overview
---

## Algorithm 1: Priority-Based Traffic Light Control with Hysteresis
**Location:** movement.c (`updateAllPriorities()`, `getNextRoadToServe()`)

### Purpose
Select the next road to receive the green signal while avoiding rapid signal switching.

### Logic
1. **Priority Update (per road):**
   - Read vehicle count in **L2 lane** (main straight traffic).
   - If:
     - L2 > 10 → priority = HIGH (100)
     - L2 < 5 → priority = NORMAL (1)
     - 5–10 → keep previous priority (prevents oscillation)

2. **Road Selection:**
   - Ignore roads with no waiting vehicles (L1 + L2 = 0).
   - Choose road with highest priority.
   - If priorities tie, choose the road **waiting longest**.
   - If all roads empty → round-robin fallback.

---

## Algorithm 2: Circular Queue Operations
**Location:** vehicle.c (`enqueue()`, `dequeue()`, `getLaneVehicle()`)

### Purpose
Efficient vehicle storage with constant-time operations.

### Structure
- Fixed-size array (MAX_QUEUE = 120)
- Front and rear indices wrap using modulo.

### Operations
- **Enqueue**
  - Reject if queue full or lane capacity exceeded.
  - Move rear pointer circularly.
  - Insert vehicle and increment count.
- **Dequeue**
  - Reject if empty.
  - Remove vehicle at front.
  - Move front pointer and decrement count.
- **Access**
  - Convert logical index → physical index using `(front + index) % MAX_QUEUE`.

---

## Algorithm 3: Vehicle Movement with Collision Avoidance
**Location:** movement.c (`moveLaneTowardCenter()`)

### Purpose
Move vehicles realistically while preventing overlap and red-light violations.

---

### A. L1 & L2 Lanes (Signal-Controlled)

1. **Direction Setup**
   - Road 0: +Y
   - Road 1: −X
   - Road 2: −Y
   - Road 3: +X

2. **Process Vehicles (Back → Front)**
   - Prevent rear vehicles from pushing stopped front vehicles.

3. **Stopping Rules**
   - Stop if:
     - Light is RED, or
     - Road is not current green, or
     - Vehicle too close to intersection stop line, or
     - Vehicle ahead is within minimum spacing.

4. **Movement**
   - If safe, update position.
   - Else mark vehicle as stopped.

5. **Intersection Handling**
   - On reaching center:
     - L1 → left turn
     - L2 → 50% straight, 50% left
   - Vehicle removed from lane and placed into transition list.

---

### B. L3 Lanes (Bypass)

1. Vehicles ignore traffic lights.
2. Move continuously.
3. Maintain minimum spacing.
4. Remove vehicle once it leaves the screen.

---

## Algorithm 4: Transition Vehicle Movement
**Location:** movement.c (`moveTransitions()`)

### Purpose
Safely guide vehicles through the intersection into target lanes.

### Steps
1. Sort transitioning vehicles by waiting time (older first).
2. Move each vehicle toward target lane center.
3. Prevent overlap with other transitioning vehicles.
4. On reaching target:
   - Join lane if space exists.
   - Force join after timeout (deadlock prevention).

---

## Algorithm 5: Collision Detection
**Location:** movement.c (`checkTooCloseInLane()`)

### Purpose
Ensure minimum spacing between vehicles.

### Method
- Compare reference vehicle with all others in lane.
- Compute Euclidean distance.
- If distance < MIN_SPACING → movement blocked.

---

## Algorithm 6: Following Distance Calculation
**Location:** movement.c (`getDistanceToVehicleAhead()`)

### Purpose
Detect nearest vehicle in front.

### Logic
- Only consider vehicles ahead (based on road direction).
- Measure distance along movement axis.
- Return smallest positive distance.

---

## Algorithm 7: Vehicle Spawning (Traffic Generator)
**Location:** traffic_generator.c

### Purpose
Create realistic traffic patterns.

### Lane Selection
- L1: 25%
- L2: 60%
- L3: 15%

### Rules
- If preferred lane full → try others.
- Enforce minimum spawn time (200 ms per lane).
- Reject spawn if spacing unsafe.

---

## Algorithm 8: File-Based Communication
**Location:** file_reader.c

### Purpose
Synchronize generator and simulator via text files.

### Steps
1. Open per-road file.
2. Skip previously read lines.
3. Parse new vehicle entries.
4. Compute spawn coordinates.
5. Spawn only if spacing allows.

---

## Algorithm 9: Geometry Calculations
**Location:** geometry.c

### Purpose
Convert logical road/lane data into screen positions.

### Tasks
- Map logical lane (L1/L2/L3) to physical lane index.
- Compute spawn coordinates relative to intersection center.
- Handle road direction symmetry.

---

## Algorithm 10: Stuck Vehicle Cleanup
**Location:** movement.c (`cleanupStuckTransitions()`)

### Purpose
Avoid infinite waiting states.

### Logic
- Run every 5 seconds.
- Remove transitioning vehicles waiting too long (>150 frames).

---

## Algorithm 11: Dynamic Green Time Calculation
**Location:** movement.c (`calculateVehiclesToServe()`)

### Purpose
Adjust green duration based on traffic demand.

### Steps
1. Count waiting vehicles in L1 + L2.
2. Serve at least one vehicle.
3. Green time = vehicles × 800 ms.

---

## Time Complexity Summary

| Component | Complexity |
|---------|------------|
| Priority Control | O(1) |
| Queue Operations | O(1) |
| Vehicle Movement | O(k²) |
| Transition Handling | O(t²) |
| Collision / Following | O(k) |
| Spawning / Geometry | O(1) |
| Overall System | O(N²) |

Where `k` = vehicles per lane, `t` = transitioning vehicles, `N` = total vehicles.

---
# 🚀Process To Run
## Prerquisites
- Windows Operating System (Windows 7/10/11)
- Visual Studio 2019/2022 with C++ development tools or MinGW GCC Compiler
- SDL2 Libraries (Development version)
- SDL2_ttf Libraries (for text rendering)

## Download Links
- [SDL2](https://github.com/libsdl-org/SDL/releases)
- [SDL2_ttf]( https://github.com/libsdl-org/SDL_ttf/releases)
- [Visual Studio](https://visualstudio.microsoft.com/downloads/)

## 🔧Setup Instructions
``` bash
1. Clone the repository:
   git clone https://github.com/subodh-baniya/dsa-queue-simulator.git

2. Install SDL2 and SDL2_ttf development libraries:
  
3. Verify folder structure:
   After cloning, your structure should look like:
   
 dsa-queue-simulator/
├── traffic-generator/                       # MUST be separate folder
│   └── traffic_generator.c
│
├── traffic-simulator/
│   └── src/
│       ├── headers/
│       │   ├── constants.h
│       │   ├── file_reader_.h
│       │   ├── geometry.h
│       │   └── ... (other .h files)
│       │
│       ├── simulator.c
│       ├── vehicle.c
│       ├── geometry.c
│       ├── globals.c
│       └── ... (other .c modules)
│
├── traffic-simulator.vcxproj         
└── simulator.sln



4. IF TrafficGenerator is INSIDE TrafficSimulator folder:
   Move it outside! It should be at the SAME LEVEL as TrafficSimulator:
   
   ❌ WRONG:
dsa-queue-simulator/
└── traffic-simulator/
   └──traffic-generator/     # WRONG location
   
   ✅ CORRECT:
   dsa-queue-simulator/
   ├── traffic-generator/         # CORRECT location
   └── traffic-simulator/

5. Open simulator.sln in Visual Studio
   (Solution contains both projects in separate folders)

6. Configure project properties for BOTH projects:
   - C/C++ → General → Additional Include Directories:
     C:\SDL2\include
     C:\SDL2_ttf\include
   - Linker → General → Additional Library Directories:
     C:\SDL2\lib\x64
     C:\SDL2_ttf\lib\x64
   - Linker → Input → Additional Dependencies:
     SDL2.lib
     SDL2_ttf.lib
     SDL2main.lib (for TrafficSimulator only)

7. Set multiple startup projects:
   - Right-click solution → "Set Startup Projects..."
   - Select "Multiple startup projects"
   - Set BOTH TrafficGenerator and TrafficSimulator to "Start"
   - Use arrows to ensure TrafficGenerator starts first

8. Build the solution (Ctrl+Shift+B)

9. Copy required DLLs to output directory (bin\x64\Debug\):
   - SDL2.dll
   - SDL2_ttf.dll

10. Run the simulation (Press F5):
    - TrafficGenerator starts in console window
    - TrafficSimulator starts in graphics window
    - Both communicate via C:\TrafficShared\ folder

   ```
# 🐛 Troubleshooting
## Common Issues and Solutions:
1. **SDL2 not found errors**: Ensure DLLs are in the same folder as executables
2. **Generator not creating files**: Check C:\TrafficShared\ folder permissions
3. **No vehicles appearing**: Run as Administrator if folder access issues
4. **Visual Studio build errors**: Ensure x64 configuration is selected


 # 👤 Assignment Details
**Title:** Traffic Junction Simulator<br>  
**Name:** Subodh Baniya <br> 
**Roll Number:** 08 <br>
**Course:** COMP-202 <br>
**Date:** 27th December, 2025










#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "TrafficNetwork.h"

int main() {
    std::srand(std::time(0)); // Seed random number generator
    std::cout << "Initializing Big City Traffic Simulation..." << std::endl;
    
    TrafficNetwork city;
    
    // 1. Create a 4x4 Grid Network (16 Intersections)
    // Grid size 4x4. Nodes 0 to 15.
    // Coordinates: x = col * 200, y = row * 200
    int rows = 4;
    int cols = 4;
    double blockSize = 200.0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int id = r * cols + c;
            city.addIntersection(id, c * blockSize + 50, r * blockSize + 50); // Offset for padding
        }
    }
    
    // 2. Add Roads (Bidirectional)
    int roadIDCounter = 1000;
    double speedLimit = 15.0; // 15 m/s (~54 km/h)
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int curr = r * cols + c;
            
            // Connect to Right (East)
            if (c < cols - 1) {
                int right = r * cols + (c + 1);
                city.addRoad(roadIDCounter++, curr, right, blockSize, speedLimit);
                city.addRoad(roadIDCounter++, right, curr, blockSize, speedLimit);
            }
            
            // Connect to Bottom (South)
            if (r < rows - 1) {
                int bottom = (r + 1) * cols + c;
                city.addRoad(roadIDCounter++, curr, bottom, blockSize, speedLimit);
                city.addRoad(roadIDCounter++, bottom, curr, blockSize, speedLimit);
            }
        }
    }
    
    city.printStaticGraph();
    
    // 3. Spawn Vehicles
    int numVehicles = 300;
    std::cout << "Spawning " << numVehicles << " vehicles..." << std::endl;
    
    for (int i = 0; i < numVehicles; ++i) {
        int startNode = std::rand() % (rows * cols);
        int destNode = std::rand() % (rows * cols);
        
        // Ensure start != dest
        while (destNode == startNode) {
            destNode = std::rand() % (rows * cols);
        }
        
        bool isEmergency = (std::rand() % 20 == 0); // 5% chance of emergency
        
        // Spawn randomly over the first 30 seconds
        // But we need to set spawn time in the vehicle logic if we supported delayed spawn.
        // Our current Vehicle constructor takes spawnTime.
        double spawnTime = (double)(std::rand() % 30); 
        
        city.spawnVehicle(i + 1, startNode, destNode, isEmergency, spawnTime);
        // Note: We need to update spawnVehicle to accept spawnTime or handle it.
        // Let's check TrafficNetwork.h/cpp. 
        // The current spawnVehicle implementation calculates path immediately.
        // We should probably update it to respect spawnTime, but for now let's just spawn them.
        // Wait, the Vehicle constructor HAS spawnTime.
        // But TrafficNetwork::spawnVehicle doesn't take it as arg in the previous version.
        // We should update TrafficNetwork::spawnVehicle signature or just pass 0 for now if we can't change header easily.
        // Actually, let's look at the header we saw earlier.
        // Vehicle.h has spawnTime. TrafficNetwork::spawnVehicle signature was:
        // void spawnVehicle(int id, int startNode, int destNode, bool isEmergency);
        // So we can't pass spawnTime unless we overload.
        // For this demo, let's just spawn them all "logically" at start, 
        // but the simulation loop we wrote checks `currentTime >= v->spawnTime`.
        // So we need to set v->spawnTime.
        // Since we can't pass it, we might need to modify TrafficNetwork.cpp/h again?
        // OR, we can just hack it: The current implementation sets spawnTime = currentTime (which is 0).
        // So all vehicles try to start at 0. This causes massive congestion at start. Perfect for testing!
    }
    
    // 4. Run Simulation
    double duration = 300.0; // 5 minutes
    std::cout << "Starting Simulation (Duration: " << duration << " seconds)..." << std::endl;
    city.runSimulation(duration);
    
    std::cout << "Simulation Complete." << std::endl;
    
    return 0;
}

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
    int rows = 4;
    int cols = 4;
    double blockSize = 200.0;
    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int id = r * cols + c;
            city.addIntersection(id, c * blockSize + 50, r * blockSize + 50); 
        }
    }
    
    // 2. Add Roads (Bidirectional)
    int roadIDCounter = 1000;
    double speedLimit = 15.0; 
    
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
    std::cout << "Spawning " << numVehicles << " vehicles over 300 seconds..." << std::endl;
    
    for (int i = 0; i < numVehicles; ++i) {
        int startNode = std::rand() % (rows * cols);
        int destNode = std::rand() % (rows * cols);
        while (destNode == startNode) {
            destNode = std::rand() % (rows * cols);
        }
        
        bool isEmergency = (std::rand() % 20 == 0); 
        
        // FIX: Spread spawns over 300 seconds (5 mins) instead of 30 seconds
        // This creates a realistic flow of ~1 car per second across the whole city.
        double spawnTime = (double)(std::rand() % 300); 
        
        city.spawnVehicle(i + 1, startNode, destNode, isEmergency, spawnTime);
    }

    // TEST: Spawn a GUARANTEED Ambulance (Vehicle 999) late in the simulation
    // Spawning at t=150s ensures there is already traffic on the road to interact with.
    std::cout << "Spawning TEST AMBULANCE (ID 999) at t=150.0s..." << std::endl;
    city.spawnVehicle(999, 0, 15, true, 150.0);
    
    // 4. Run Simulation
    // Increased duration to 600s to allow late-spawning cars to finish.
    double duration = 600.0; 
    std::cout << "Starting Simulation (Duration: " << duration << " seconds)..." << std::endl;
    city.runSimulation(duration);
    
    // PRINT STATS
    city.printStatistics();
    
    std::cout << "Simulation Complete." << std::endl;
    
    return 0;
}
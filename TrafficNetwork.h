#ifndef TRAFFICNETWORK_H
#define TRAFFICNETWORK_H

#include <unordered_map>
#include <queue>
#include <vector>
#include <iostream>
#include <functional>
#include "Intersection.h"
#include "Vehicle.h"
#include "Event.h"
#include "Road.h"

class TrafficNetwork {
private:
    std::unordered_map<int, Intersection*> intersections;
    std::unordered_map<int, Vehicle*> vehicles;
    std::vector<Road*> roads; // Keep track of all roads to free memory
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue;
    
    double currentTime;

public:
    TrafficNetwork();
    ~TrafficNetwork();

    // Graph Construction
    // Graph Construction
    void addIntersection(int id, double x = 0, double y = 0);
    void addRoad(int id, int source, int dest, double length, double speedLimit);

    // Visualization Support
    void printStaticGraph();
    void printNetworkState();

    // Simulation Control
    void scheduleEvent(double time, EventType type, int entityID, int secondaryID = -1);
    void runSimulation(double duration);
    void processEvent(const Event& event);
    void resetVehicle(Vehicle* v);

    // Vehicle Management
    void spawnVehicle(int id, int startNode, int destNode, bool isEmergency, double spawnTime);
    
    // Algorithms
    std::vector<int> calculateShortestPath(int startNode, int destNode); // Dijkstra
};

#endif // TRAFFICNETWORK_H

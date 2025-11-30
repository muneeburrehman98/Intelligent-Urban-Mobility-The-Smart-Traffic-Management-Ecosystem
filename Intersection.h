#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include <algorithm>
#include "Road.h"

class Intersection {
public:
    int id;
    double x, y; // Coordinates for visualization
    std::vector<Road*> incomingRoads;
    std::vector<Road*> outgoingRoads;
    
    // Traffic Light State
    int greenLightRoadIndex; // Index in incomingRoads that currently has green
    double lastLightChangeTime;

    Intersection(int id, double x = 0, double y = 0) : id(id), x(x), y(y), greenLightRoadIndex(-1), lastLightChangeTime(0.0) {}

    void addIncomingRoad(Road* road) {
        incomingRoads.push_back(road);
    }

    void addOutgoingRoad(Road* road) {
        outgoingRoads.push_back(road);
    }

    // Greedy Algorithm for Traffic Light
    // Returns the ID of the road that should get Green light next
    int decideNextGreenLight(double currentTime);
};

#endif // INTERSECTION_H

#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
#include <stack>
#include <vector>

class Vehicle {
public:
    int id;
    int currentIntersectionID;
    int destinationID;
    bool isEmergency;
    double spawnTime;
    double arrivalTime;
    std::vector<int> path; // Stores the sequence of Intersection IDs
    int pathIndex; // Current position in the path

    double currentPosition; // Distance from start of current road
    double speed;
    bool isMoving;
    double length;
    double minGap;

    Vehicle(int id, int startNode, int endNode, bool emergency, double spawnTime)
        : id(id), currentIntersectionID(startNode), destinationID(endNode), 
          isEmergency(emergency), spawnTime(spawnTime), arrivalTime(-1.0), pathIndex(0),
          currentPosition(0.0), speed(0.0), isMoving(false), length(4.0), minGap(2.0) {}

    // Methods to be implemented in .cpp
    void setPath(const std::vector<int>& newPath);
    int getNextIntersection() const;
};

#endif // VEHICLE_H

#ifndef ROAD_H
#define ROAD_H

#include <cmath>
#include <deque>
#include "Vehicle.h"

struct Road {
    int id;
    int sourceID;
    int destinationID;
    double baseDistance;
    double speedLimit;
    int currentVehicleCount;
    int capacity; // To calculate congestion factor
    std::deque<Vehicle*> vehicleQueue; // Queue of vehicles on this road

    double getQueueLength() const {
        return vehicleQueue.size();
    }

    Road(int id, int src, int dest, double dist, double speed, int cap = 10)
        : id(id), sourceID(src), destinationID(dest), baseDistance(dist), speedLimit(speed), currentVehicleCount(0), capacity(cap) {}

    double getCongestionFactor() const {
        if (capacity == 0) return 0.0;
        double factor = (double)currentVehicleCount / capacity;
        return (factor > 0.99) ? 0.99 : factor; // Cap at 0.99 to avoid division by zero
    }

    double getDynamicWeight() const {
        double congestion = getCongestionFactor();
        // Weight = Distance / (Speed * (1 - Congestion))
        // If congestion is high, effective speed drops, weight increases.
        return baseDistance / (speedLimit * (1.0 - congestion));
    }
};

#endif // ROAD_H

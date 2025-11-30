#include "Intersection.h"
#include <limits>
#include <iostream>
#include <algorithm>

int Intersection::decideNextGreenLight(double currentTime) {
    // Smart Traffic Light Logic:
    // Find the incoming road with the longest queue of vehicles waiting.
    
    int bestRoadIndex = -1;
    double maxQueue = -1.0;

    for (int i = 0; i < incomingRoads.size(); ++i) {
        double qLen = incomingRoads[i]->getQueueLength();
        if (qLen > maxQueue) {
            maxQueue = qLen;
            bestRoadIndex = i;
        }
    }

    // If all queues are empty, keep current or switch round-robin?
    // For now, if empty, just keep current or pick first.
    if (maxQueue == 0 && greenLightRoadIndex != -1) {
        return incomingRoads[greenLightRoadIndex]->id;
    }

    if (bestRoadIndex != -1) {
        greenLightRoadIndex = bestRoadIndex;
        lastLightChangeTime = currentTime;
        return incomingRoads[bestRoadIndex]->id;
    }
    
    return -1;
}

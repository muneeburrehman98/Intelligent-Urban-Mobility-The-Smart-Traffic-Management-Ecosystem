#include "Intersection.h"
#include <limits>
#include <iostream>
#include <algorithm>

int Intersection::decideNextGreenLight(double currentTime) {
    // 1. EMERGENCY PRIORITY CHECK
    // Scan all incoming roads for emergency vehicles
    for (Road* r : incomingRoads) {
        for (Vehicle* v : r->vehicleQueue) {
            if (v->isEmergency) {
                // Determine index for updating state
                for(int i=0; i<incomingRoads.size(); ++i) {
                    if(incomingRoads[i]->id == r->id) {
                        // SILENCED DEBUG PRINT FOR STATS REPORT
                        // if (greenLightRoadIndex != i) {
                        //    std::cout << "[t=" << currentTime << "] !!! EMERGENCY OVERRIDE !!! at Intersection " 
                        //              << id << " for Ambulance V" << v->id << " on Road " << r->id << std::endl;
                        // }
                        greenLightRoadIndex = i;
                        break;
                    }
                }
                lastLightChangeTime = currentTime;
                return r->id;
            }
        }
    }

    // 2. ROUND ROBIN LOGIC (Fairness)
    int numRoads = incomingRoads.size();
    if (numRoads == 0) return -1;

    int startIndex = (greenLightRoadIndex + 1) % numRoads;

    for (int i = 0; i < numRoads; ++i) {
        int idx = (startIndex + i) % numRoads;
        if (incomingRoads[idx]->getQueueLength() > 0) {
            greenLightRoadIndex = idx;
            lastLightChangeTime = currentTime;
            return incomingRoads[idx]->id;
        }
    }

    // 3. FALLBACK
    greenLightRoadIndex = startIndex;
    lastLightChangeTime = currentTime;
    return incomingRoads[startIndex]->id;
}
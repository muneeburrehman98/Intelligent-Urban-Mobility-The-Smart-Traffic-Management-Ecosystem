#include "Vehicle.h"

void Vehicle::setPath(const std::vector<int>& newPath) {
    path = newPath;
    pathIndex = 0; // Reset progress
}

int Vehicle::getNextIntersection() const {
    if (pathIndex + 1 < path.size()) {
        return path[pathIndex + 1];
    }
    return -1; // Reached destination or no path
}

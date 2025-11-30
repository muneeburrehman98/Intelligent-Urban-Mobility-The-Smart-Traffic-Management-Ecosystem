#ifndef EVENT_H
#define EVENT_H

#include <string>

enum EventType {
    VEHICLE_SPAWN,
    VEHICLE_ARRIVAL,
    LIGHT_CHANGE,
    PATH_RECALCULATION
};

struct Event {
    double timestamp;
    EventType type;
    int entityID; // Can be Vehicle ID or Intersection ID depending on type
    int secondaryID; // Optional, e.g., Road ID or Destination ID

    // Priority Queue needs to order by smallest timestamp first (Min-Heap)
    bool operator>(const Event& other) const {
        return timestamp > other.timestamp;
    }
};

#endif // EVENT_H

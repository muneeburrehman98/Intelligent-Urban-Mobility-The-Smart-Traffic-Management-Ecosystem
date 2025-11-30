#include "TrafficNetwork.h"
#include <limits>
#include <algorithm>

TrafficNetwork::TrafficNetwork() : currentTime(0.0) {}

TrafficNetwork::~TrafficNetwork() {
    for (auto& pair : intersections) delete pair.second;
    for (auto& pair : vehicles) delete pair.second;
    for (auto* road : roads) delete road;
}

void TrafficNetwork::addIntersection(int id, double x, double y) {
    if (intersections.find(id) == intersections.end()) {
        intersections[id] = new Intersection(id, x, y);
    }
}

void TrafficNetwork::printStaticGraph() {
    std::cout << "NODES" << std::endl;
    for (auto& pair : intersections) {
        std::cout << pair.second->id << " " << pair.second->x << " " << pair.second->y << std::endl;
    }
    std::cout << "EDGES" << std::endl;
    for (Road* r : roads) {
        std::cout << r->id << " " << r->sourceID << " " << r->destinationID << std::endl;
    }
    std::cout << "END_GRAPH" << std::endl;
}

void TrafficNetwork::printNetworkState() {
    std::cout << "STATE " << currentTime << std::endl;
    for (auto& pair : vehicles) {
        Vehicle* v = pair.second;
        if (v->isMoving) {
            // Output: V ID RoadID Position
            // We need to find the Road ID based on path[pathIndex] -> path[pathIndex+1]
            // This is a bit inefficient to search every time, but fine for this scale.
            int u = v->path[v->pathIndex];
            int nextNode = v->path[v->pathIndex + 1];
            // Find road ID
            int roadID = -1;
            if (intersections.find(u) != intersections.end()) {
                for (Road* r : intersections[u]->outgoingRoads) {
                    if (r->destinationID == nextNode) {
                        roadID = r->id;
                        break;
                    }
                }
            }
            std::cout << "V " << v->id << " " << roadID << " " << v->currentPosition << std::endl;
        }
    }
    // Also output traffic lights
    for (auto& pair : intersections) {
        Intersection* i = pair.second;
        int greenRoadId = -1;
        if (i->greenLightRoadIndex != -1 && i->greenLightRoadIndex < i->incomingRoads.size()) {
            greenRoadId = i->incomingRoads[i->greenLightRoadIndex]->id;
        }
        std::cout << "L " << i->id << " " << greenRoadId << std::endl;
    }
    std::cout << "END_STATE" << std::endl;
}

void TrafficNetwork::addRoad(int id, int source, int dest, double length, double speedLimit) {
    Road* newRoad = new Road(id, source, dest, length, speedLimit);
    roads.push_back(newRoad);
    
    if (intersections.find(source) != intersections.end()) {
        intersections[source]->addOutgoingRoad(newRoad);
    }
    if (intersections.find(dest) != intersections.end()) {
        intersections[dest]->addIncomingRoad(newRoad);
    }
}

void TrafficNetwork::scheduleEvent(double time, EventType type, int entityID, int secondaryID) {
    eventQueue.push({time, type, entityID, secondaryID});
}

void TrafficNetwork::spawnVehicle(int id, int startNode, int destNode, bool isEmergency, double spawnTime) {
    Vehicle* v = new Vehicle(id, startNode, destNode, isEmergency, spawnTime);
    vehicles[id] = v;
    
    // Calculate initial path
    std::vector<int> path = calculateShortestPath(startNode, destNode);
    v->setPath(path);
    
    // Schedule first arrival event (at the next intersection)
    // For simplicity, we assume the vehicle starts AT the startNode intersection
    // and immediately enters the first road of its path.
    
    if (path.size() > 1) {
        int nextNode = path[1];
        // Find the road connecting startNode to nextNode
        Road* roadToTake = nullptr;
        for (Road* r : intersections[startNode]->outgoingRoads) {
            if (r->destinationID == nextNode) {
                roadToTake = r;
                break;
            }
        }
        
        if (roadToTake) {
            roadToTake->currentVehicleCount++;
            // Calculate travel time = Distance / Speed
            // We use static speed limit for initial estimate, or dynamic?
            // Let's use dynamic weight logic but for time: Time = Distance / (Speed * (1-Congestion))
            double travelTime = roadToTake->baseDistance / roadToTake->speedLimit; // Simplified
            
            scheduleEvent(currentTime + travelTime, VEHICLE_ARRIVAL, id, nextNode);
        }
    }
}

std::vector<int> TrafficNetwork::calculateShortestPath(int startNode, int destNode) {
    // Dijkstra's Algorithm
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    
    for (auto& pair : intersections) {
        dist[pair.first] = std::numeric_limits<double>::infinity();
    }
    dist[startNode] = 0.0;
    
    // Priority Queue for Dijkstra: <Distance, NodeID>
    // Use std::greater for Min-Heap
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pq;
    pq.push({0.0, startNode});
    
    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (d > dist[u]) continue;
        if (u == destNode) break; // Optimization
        
        if (intersections.find(u) == intersections.end()) continue;

        for (Road* road : intersections[u]->outgoingRoads) {
            int v = road->destinationID;
            double weight = road->getDynamicWeight();
            
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    // Reconstruct path
    std::vector<int> path;
    int curr = destNode;
    if (dist[curr] == std::numeric_limits<double>::infinity()) {
        return path; // No path found
    }
    
    while (curr != startNode) {
        path.push_back(curr);
        if (prev.find(curr) == prev.end()) return {}; // Should not happen if path exists
        curr = prev[curr];
    }
    path.push_back(curr); // Add start node
    std::reverse(path.begin(), path.end());
    return path;
}



void TrafficNetwork::resetVehicle(Vehicle* v) {
    // Pick random start and end nodes
    int numNodes = intersections.size();
    if (numNodes < 2) return;

    int startNode = std::rand() % numNodes;
    int destNode = std::rand() % numNodes;
    while (destNode == startNode) {
        destNode = std::rand() % numNodes;
    }

    v->currentIntersectionID = startNode;
    v->destinationID = destNode;
    v->path = calculateShortestPath(startNode, destNode);
    v->pathIndex = 0;
    v->currentPosition = 0.0;
    v->isMoving = false; // Set to false so the spawn logic picks it up
    v->spawnTime = currentTime; // Ready to spawn immediately
    v->arrivalTime = -1.0;
}

void TrafficNetwork::runSimulation(double duration) {
    double timeStep = 0.1; // 100ms per step
    
    // Initial events
    for (auto& pair : intersections) {
        scheduleEvent(0.0, LIGHT_CHANGE, pair.first);
    }
    
    while (currentTime < duration) {
        // 1. Process Events (Traffic Lights)
        while (!eventQueue.empty() && eventQueue.top().timestamp <= currentTime) {
            Event e = eventQueue.top();
            eventQueue.pop();
            processEvent(e);
        }

        // 2. Spawn Vehicles (Check spawnTime)
        for (auto& pair : vehicles) {
            Vehicle* v = pair.second;
            if (!v->isMoving && v->currentPosition == 0 && v->pathIndex == 0 && v->arrivalTime < 0) {
                 if (currentTime >= v->spawnTime) {
                     // Try to enter first road
                     int u = v->path[v->pathIndex];
                     int nextNode = v->path[v->pathIndex + 1];
                     
                     // Find road
                     Road* startRoad = nullptr;
                     if (intersections.find(u) != intersections.end()) {
                         for (Road* r : intersections[u]->outgoingRoads) {
                             if (r->destinationID == nextNode) {
                                 startRoad = r;
                                 break;
                             }
                         }
                     }
                     
                     if (startRoad) {
                         // Check if space available at start of road
                         // Last vehicle in queue must be > length + gap
                         bool spaceAvailable = true;
                         if (!startRoad->vehicleQueue.empty()) {
                             Vehicle* last = startRoad->vehicleQueue.back();
                             if (last->currentPosition < (v->length + v->minGap)) {
                                 spaceAvailable = false;
                             }
                         }
                         
                         if (spaceAvailable) {
                             v->isMoving = true;
                             v->currentPosition = 0.0; // Start at 0
                             startRoad->vehicleQueue.push_back(v);
                         }
                     }
                 }
            }
        }

        // 3. Update Vehicles (Physics & Queues)
        for (Road* r : roads) {
            if (r->vehicleQueue.empty()) continue;

            for (size_t i = 0; i < r->vehicleQueue.size(); ++i) {
                Vehicle* v = r->vehicleQueue[i];
                double moveDist = 10.0 * timeStep; // Speed 10 m/s
                double limit = r->baseDistance; // Default limit is end of road
                
                if (i == 0) {
                    // Front of queue
                    Intersection* dest = intersections[r->destinationID];
                    bool isGreen = false;
                    if (dest->greenLightRoadIndex != -1 && 
                        dest->incomingRoads[dest->greenLightRoadIndex]->id == r->id) {
                        isGreen = true;
                    }
                    
                    if (!isGreen) {
                        limit = r->baseDistance;
                    } else {
                        limit = r->baseDistance + 100.0; 
                    }
                } else {
                    // Following another car
                    Vehicle* leader = r->vehicleQueue[i-1];
                    limit = leader->currentPosition - leader->length - v->minGap;
                }
                
                double newPos = v->currentPosition + moveDist;
                if (newPos > limit) newPos = limit;
                v->currentPosition = newPos;
            }
            
            // Check if Front car exits road
            if (!r->vehicleQueue.empty()) {
                Vehicle* front = r->vehicleQueue.front();
                if (front->currentPosition >= r->baseDistance) {
                    Intersection* dest = intersections[r->destinationID];
                    bool isGreen = (dest->greenLightRoadIndex != -1 && 
                                    dest->incomingRoads[dest->greenLightRoadIndex]->id == r->id);
                                    
                    if (isGreen) {
                        // Move to next road
                        if (front->pathIndex + 1 < front->path.size() - 1) {
                            int u = front->path[front->pathIndex + 1];
                            int w = front->path[front->pathIndex + 2];
                            
                            Road* nextRoad = nullptr;
                            if (intersections.find(u) != intersections.end()) {
                                for (Road* nr : intersections[u]->outgoingRoads) {
                                    if (nr->destinationID == w) {
                                        nextRoad = nr;
                                        break;
                                    }
                                }
                            }
                            
                            if (nextRoad) {
                                bool space = true;
                                if (!nextRoad->vehicleQueue.empty()) {
                                    Vehicle* last = nextRoad->vehicleQueue.back();
                                    if (last->currentPosition < (front->length + front->minGap)) {
                                        space = false;
                                    }
                                }
                                
                                if (space) {
                                    r->vehicleQueue.pop_front();
                                    front->pathIndex++;
                                    front->currentPosition = 0.0;
                                    nextRoad->vehicleQueue.push_back(front);
                                } else {
                                    front->currentPosition = r->baseDistance;
                                }
                            }
                        } else {
                            // Reached Destination -> RECYCLE
                            r->vehicleQueue.pop_front();
                            resetVehicle(front);
                        }
                    }
                }
            }
        }

        // 4. Output State (Snapshot)
        static double lastPrint = 0.0;
        if (currentTime - lastPrint >= 0.5) {
            printNetworkState();
            lastPrint = currentTime;
        }

        currentTime += timeStep;
    }
}



void TrafficNetwork::processEvent(const Event& event) {
    if (event.type == LIGHT_CHANGE) {
        int intersectionID = event.entityID;
        if (intersections.find(intersectionID) != intersections.end()) {
            int greenRoadID = intersections[intersectionID]->decideNextGreenLight(currentTime);
            // Schedule next check
            scheduleEvent(currentTime + 5.0, LIGHT_CHANGE, intersectionID);
        }
    }
}

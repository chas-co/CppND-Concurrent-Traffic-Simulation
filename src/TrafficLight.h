#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

using namespace std;

// forward declarations to avoid include cycle
class Vehicle;

// Define phases of the traffic light red and green
enum TrafficLightPhase {red, green};
 
/* Message Queue class with public methods send and receive*/
template <class T>
class MessageQueue
{
public:
    // send take an rvalue reference 
    void send(T &&message);

    // receive returns the template type T
    T receive();

private:
    deque <TrafficLightPhase> _queue; 
    condition_variable _condition; 
    mutex _mqMutex;
    
};

// Traffic light defined as a child class of traffic object
class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();
    
    

private:
    // typical behaviour methods
    void cycleThroughPhases();
    int RandomNumber(int min, int max);

    TrafficLightPhase _currentPhase; // stores the phase of the traffic light red/green
    mutex _mutex;
    shared_ptr<MessageQueue<TrafficLightPhase>> _tlpMessages;
};

#endif
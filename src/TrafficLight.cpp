#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include <stdlib.h>
#include <thread>
#include "TrafficLight.h"

using namespace std;

/* Implementation of class "MessageQueue" */

template <typename T>
void MessageQueue<T>::send(T &&message)
{    
    // make the changes under a lock to prevent data races
    lock_guard<mutex> lock(_mqMutex);

    // add the message to the queue
    _queue.push_back(move(message));

    // notify client after pushing the message into queue
    _condition.notify_one();
}

template <typename T>
T MessageQueue<T>::receive()
{
    // Perform the operation under lock
    unique_lock<mutex> ulock(_mqMutex);

    // Wait until there is a message in the queue
    _condition.wait(ulock,[this]{return ! _queue.empty();});

    // Pull the message from the queue using move semantics 
    T message = std::move(_queue.back());
    _queue.clear();

    return message;
}


/* Implementation of class "TrafficLight" */

// Constructor for the traffic light object
TrafficLight::TrafficLight()
{
    // initialise the traffic light to red 
    _currentPhase = TrafficLightPhase::red;

    //initialize the message queue as a shared pointer to enable access by multiple threads.
    _tlpMessages = make_shared<MessageQueue<TrafficLightPhase>>();
}

// Wait till the traffic light turns green then return from the method
void TrafficLight::waitForGreen()
{
    while (true)
    {
        // Sleep/wait for 1ms between loops to reduce CPU usage
        this_thread::sleep_for(chrono::milliseconds(1));

        // call the receive method on the MessageQueue object
        TrafficLightPhase receivePhase = _tlpMessages->receive();

        //check if the traffic light is green and return 
        if(receivePhase == TrafficLightPhase::green)
        {
            return;
        }
    }
    
}

// Return the current phase of the traffic light 
TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

// simulate the traffic light object
void TrafficLight::simulate()
{
    //  Cycle through phase is started in a thread when the simulate method is called.
    threads.emplace_back(thread(&TrafficLight::cycleThroughPhases,this));
}

/* virtual function which is executed in a thread. Measures the time between two loop cycles 
and toggles the current phase of the traffic light between red and green*/
void TrafficLight::cycleThroughPhases()
{
    // Initialise timing variables 
    double cycleDuration = TrafficLight::RandomNumber(4000,6000);
    auto lastUpdate = chrono::system_clock::now();

    // Infinite loop to continuously cycle through phases
    while (true)
    {
        // sleep/wait for 1ms  between two loop to reduce the CPU usage of the infinite loop
        std::this_thread::sleep_for(chrono::milliseconds(1));

        // measure the time between two loop cycles
        double timeSinceLastUpdate = chrono::duration_cast<chrono::milliseconds>( chrono::system_clock::now() - lastUpdate ).count();

        //check if the the duration for traffic light cycle is reached and update the system.
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Tenary Operator to toggle the current phase of the traffic light between red and green
            _currentPhase == TrafficLightPhase::red  ? _currentPhase = TrafficLightPhase::green : _currentPhase = TrafficLightPhase::red;

            // send an update to the message queue using move semantics
            future<void> ftrCurrentPhase = async(launch::async, &MessageQueue<TrafficLightPhase>::send, _tlpMessages,std::move(_currentPhase));
            ftrCurrentPhase.wait();
            
            //Record the time at the end of the loop
            lastUpdate = chrono::system_clock::now();
        } 
    } 
}


/* Generates a random number between the min and max values specified, 
with the min and max values included*/
int TrafficLight::RandomNumber(int min, int max)
{
    auto seed =std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 randNumber(seed);
    return (randNumber() % (max - min + 1)) + min;
}


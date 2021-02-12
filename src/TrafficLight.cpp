#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include "TrafficLight.h"

using namespace std;

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    // Perform the operation under lock
    unique_lock<mutex> ulock(_mqMutex);

    // Wait until there is a message in the queue
    _cond.wait(ulock,[this]{return ! _queue.empty();});

    // Pull the message from the queue using move semantics 
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // make the changes under a lock to prevent data races
    lock_guard<mutex> lock(_mqMutex);

    // add the message to the queue
    _queue.push_back(move(msg));

    // notify client after pushing the message into queue
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _tlpMessages = make_shared<MessageQueue<TrafficLightPhase>>();
}
void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true)
    {
        // Sleep for 1ms every cycle to reduce CPU usage
        this_thread::sleep_for(chrono::milliseconds(1));

        // call the receive method on the MessageQueue object and check if it green
        TrafficLightPhase receivePhase = _tlpMessages->receive();
        if(receivePhase == TrafficLightPhase::green)
        {
            return;
        }
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    //Print the id of the current object and thread
    std::unique_lock<std::mutex> lckTrafficLight(_mtx);
    std::cout << "Traffic Light #" << _id << "::cycleTrhoughPhases: thread id = " << std::this_thread::get_id() << std::endl;
    lckTrafficLight.unlock();

    // Initialise timing variables 
    double cycleDuration = TrafficLight::RandomNumber(4,6);
    auto lastUpdate = std::chrono::system_clock::now();

    // Infinite loop
    while (true)
    {
        //sleep for 1ms to reduce the CPU usage of the infinite loop
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // measure the time between two loop cycles
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - lastUpdate ).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Toggle the current phase of the traffic light
            if (_currentPhase == red)
            {
                _currentPhase = green;
            }
            else
            {
                _currentPhase = red; 
            }
        }

        // send an update to the message queue 
        TrafficLightPhase msg = _currentPhase;
        future<void> ftrCurrentPhase = async(launch::async, &MessageQueue<TrafficLightPhase>::send, _tlpMessages,std::move(msg));
        //ftrCurrentPhase.wait();
        

        // Choose Randomly the cycle duration for the next cycle
       // cycleDuration = TrafficLight::RandomNumber(4000,6000);

        //reset stop watch for the next cycle
        lastUpdate = chrono::system_clock::now();
    }
    
}

/* Generates a random number between the min and max values specified, 
with the min and max values included*/
int TrafficLight::RandomNumber(int min, int max)
{
    std::random_device randNumber;
    std::mt19937 gen(randNumber());
    std::uniform_int_distribution<> uniDistr(min, max);
    return uniDistr(gen);
}


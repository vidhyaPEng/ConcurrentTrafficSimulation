#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>
#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */


template <typename TrafficLightPhase>
TrafficLightPhase MessageQueue<TrafficLightPhase>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
  std::unique_lock<std::mutex> ulock(_mutex);
	_cond.wait(ulock, [this] {return !_queue.empty();});
  TrafficLightPhase msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}
 
template <typename TrafficLightPhase>
void MessageQueue<TrafficLightPhase>::send(TrafficLightPhase &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> uLock(_mutex);
  _queue.clear();
  _queue.emplace_back(std::move(msg));
  //send notification
  _cond.notify_one(); 
}


/* Implementation of class "TrafficLight" */
//constructor
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
//destructor
TrafficLight::~TrafficLight()
{
    // set up thread barrier before this object is destroyed
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
      
      if (_trafficQueue.receive() == TrafficLightPhase::green){
        break;
      };
      
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
// auto t1 = std::chrono::high_resolution_clock::now();
  int randNum;
  std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
  while(true){
    std::this_thread::sleep_for(std::chrono::milliseconds(1));//sleep for one 

    //toggle current phase of traffic light between red adn green. 
    _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
   _trafficQueue.send(std::move(_currentPhase)); //send current phase to message queue

    //use steady clock to pause while loop execution. 
    randNum = (rand()%(3) + 4)*1000;
    next += std::chrono::milliseconds(randNum);
    std::this_thread::sleep_until(next);
}
}


#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulock(_mutexMsg);
    _condMsg.wait(ulock, [this]{return !_queueMsg.empty();});
    T msg = std::move(_queueMsg.back());
    _queueMsg.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T msg)  //send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
   // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> ulock(_mutexMsg);
  //  std::cout<< "Message " << msg << "has been added to  the queue"<< std::end;
    _queueMsg.push_back(std::move(msg));
    _condMsg.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _msgsTrafficLight = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(1){
       // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto msg  = _msgsTrafficLight->receive();
        if(msg == TrafficLightPhase::green){
         return;
         }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{   std::lock_guard<std::mutex> lg(_mutex);
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase color){
    std::lock_guard<std::mutex> lg(_mutex);
    _currentPhase = color;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread
    // when the public method „simulate“ is called. To do this, use the thread queue in the 
    //base class.
 // std::thread t(&TrafficLight::cycleThroughPhases,this) ;
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
   // t.join();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two 
    //loop cycles and toggles the current phase of the traffic light between red and green and
    // sends an update method to the message queue using move semantics. The cycle duration 
    // should be a random value between 4 and 6 seconds. Also, the while-loop should use 
    // std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4,6);
    /* std::unique_lock<std::mutex> ulock(_mutex);
     std::cout<<"TrafficLight#" << _id << "::cycleThroughPhases: thread id= "<< std::this_thread::get_id() << std::endl;
     ulock.unlock();
     */  
       int cycle_duration= distr(eng);
        auto last_time = std::chrono::system_clock::now();
    while(1){

       std::this_thread::sleep_for(std::chrono::milliseconds(1));
       auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now()- last_time).count();
        if( timeSinceLastUpdate > cycle_duration)
        {
            if( getCurrentPhase() == TrafficLightPhase::red)
            _currentPhase =  TrafficLightPhase::green;
            else
            _currentPhase = TrafficLightPhase::red ;
        
        auto msg = _currentPhase;
       /* std::future<void> isSent;
        isSent = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _msgsTrafficLight, std::move(msg)); 
        isSent.wait();
        */
       _msgsTrafficLight->send(msg);
        last_time = std::chrono::system_clock::now();
        cycle_duration = distr(eng);
        }
    }
}


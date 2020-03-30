#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "TrafficObject.h"
#include <condition_variable>
#include <deque>
#include <mutex>

// forward declarations to avoid include cycle
class Vehicle;

template <class T> class MessageQueue {
public:
  T receive();
  void send(T &&msg);

private:
  std::deque<T> _queue;
  std::condition_variable _condition;
  std::mutex _mutex;
};

enum TrafficLightPhase { red, green };

class TrafficLight : public TrafficObject {
public:
  // constructor / desctructor
  TrafficLight();

  // getter and setter
  TrafficLightPhase getCurrentPhase() const;

  // typical behaviour methods
  void waitForGreen();
  void simulate();

private:
  void cycleThroughPhases();
  std::condition_variable _condition;
  std::mutex _mutex;
  TrafficLightPhase _currentPhase;
  std::shared_ptr<MessageQueue<TrafficLightPhase>> _queue;
};

#endif
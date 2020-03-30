#include "TrafficLight.h"
#include <future>
#include <iostream>
#include <random>

template <typename T> T MessageQueue<T>::receive() {
  std::unique_lock<std::mutex> uLock(_mutex);
  _condition.wait(uLock, [this] { return !_queue.empty(); });
  T msg = std::move(_queue.back());
  _queue.pop_back();
  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  std::lock_guard<std::mutex> uLock(_mutex);
  _queue.push_back(std::move(msg));
  _condition.notify_one();
}

TrafficLight::TrafficLight() {
  _currentPhase = TrafficLightPhase::red;
  _queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    TrafficLightPhase phase = _queue->receive();
    if (phase == TrafficLightPhase::green) {
      return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() const {
  return _currentPhase;
}

void TrafficLight::simulate() {
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  std::mt19937 rng(random());
  std::uniform_int_distribution<int> gen(4, 6); // uniform, unbiased
  int cycleDuration = gen(rng);

  auto lastUpdate = std::chrono::system_clock::now();

  while (true) {
    int secondsSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(
                              std::chrono::system_clock::now() - lastUpdate)
                              .count();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (secondsSinceLastUpdate >= cycleDuration) {
      if (_currentPhase == TrafficLightPhase::green) {
        _currentPhase = TrafficLightPhase::red;
      } else {
        _currentPhase = TrafficLightPhase::green;
      }

      TrafficLightPhase msg = _currentPhase;
      std::future<void> ftr =
          std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send,
                     _queue, std::move(msg));
      ftr.wait();
      cycleDuration = gen(rng);
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}

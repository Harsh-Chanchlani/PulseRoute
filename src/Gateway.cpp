#include "Gateway.hpp"
#include <chrono>
#include <thread>

Gateway::Gateway(std::string id, double initial_success_rate, int capacity)
    : id_(std::move(id)), current_success_rate_(initial_success_rate),
      capacity_(capacity), active_requests_(0) {
  // We seed the RNG with the current time combined with the object's memory
  // address This ensures two gateways created at the exact same millisecond get
  // different seeds.
  auto time_seed =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  auto mem_seed = reinterpret_cast<std::intptr_t>(this);
  rng_.seed(time_seed ^ mem_seed);
}

void Gateway::set_success_rate(double new_rate) {
  std::lock_guard<std::mutex> lock(mutex_);
  current_success_rate_ = new_rate;
}

bool Gateway::process(const Transaction & /*tx*/) {
  // 1. Thundering Herd Mitigation: Increment active requests
  int current_active = active_requests_.fetch_add(1);

  // 2. Capacity Check: If overloaded, simulate a timeout failure
  if (current_active >= capacity_) {
    // Simulate the delay of a timeout before failing
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    active_requests_.fetch_sub(1);
    return false;
  }

  // 3. Normal Processing: Simulate standard network latency
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  bool result;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    // std::bernoulli_distribution takes the probability of returning `true`
    std::bernoulli_distribution dist(current_success_rate_);
    // Pass our rng_ engine to the distribution to get the random boolean result
    result = dist(rng_);
  }

  // Decrement active requests when done
  active_requests_.fetch_sub(1);
  return result;
}

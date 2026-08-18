#pragma once

#include <string>
#include <random>
#include <mutex>
#include "Transaction.hpp"

class Gateway {
public:
    Gateway(std::string id, double initial_success_rate, int capacity);
    
    // Processes a transaction and returns true (success) or false (failure)
    bool process(const Transaction& tx);
    
    // Allows us to simulate outages by manually changing the success rate at runtime
    void set_success_rate(double new_rate);
    
    std::string get_id() const { return id_; }
    int get_active_requests() const { return active_requests_.load(); }

private:
    std::string id_;
    double current_success_rate_;
    int capacity_;
    std::atomic<int> active_requests_;
    std::mt19937 rng_;
    std::mutex mutex_;
};

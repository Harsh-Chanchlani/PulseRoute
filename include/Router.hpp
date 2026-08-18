#pragma once

#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <random>
#include <unordered_map>
#include <iostream>
#include "Gateway.hpp"
#include "Transaction.hpp"

// A simple struct to track the historical performance of a gateway
struct GatewayStats {
    int total_requests = 0;
    int successful_requests = 0;
    
    // Optimistic Initialization: Assume a 100% success rate until we get actual data.
    // This ensures every gateway gets tried at least once at the very beginning.
    double get_success_rate() const {
        if (total_requests == 0) return 1.0; 
        return static_cast<double>(successful_requests) / total_requests;
    }
};

class Router {
public:
    Router(std::vector<std::shared_ptr<Gateway>> gateways, double epsilon, double penalty_constant);

    // Uses Epsilon-Greedy to pick a gateway
    std::shared_ptr<Gateway> route(const Transaction& tx);

    // Called by worker threads AFTER a transaction finishes to record the outcome
    void update_stats(const std::string& gateway_id, bool success);

    // Helper to print the router's internal view of the world
    void print_stats() const;

private:
    std::vector<std::shared_ptr<Gateway>> gateways_;
    std::unordered_map<std::string, GatewayStats> stats_;
    
    // The probability of exploration (e.g., 0.1 means 10% chance to explore)
    double epsilon_;
    
    // The weight we give to the active connection penalty
    double penalty_constant_;

    // std::shared_mutex allows multiple readers (route()) but exclusive writers (update_stats())
    mutable std::shared_mutex stats_mutex_;
    
    std::mt19937 rng_;
    std::mutex rng_mutex_; // Protects the random number generator
};

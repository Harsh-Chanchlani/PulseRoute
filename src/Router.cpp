#include "Router.hpp"
#include <chrono>

Router::Router(std::vector<std::shared_ptr<Gateway>> gateways, double epsilon, double penalty_constant)
    : gateways_(std::move(gateways)), epsilon_(epsilon), penalty_constant_(penalty_constant) {
    auto time_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto mem_seed = reinterpret_cast<std::intptr_t>(this);
    rng_.seed(time_seed ^ mem_seed);
}

void Router::update_stats(const std::string& gateway_id, bool success) {
    // We use a unique_lock because we are WRITING to the shared state
    std::unique_lock<std::shared_mutex> lock(stats_mutex_);
    
    GatewayStats& stat = stats_[gateway_id];
    stat.total_requests++;
    if (success) {
        stat.successful_requests++;
    }
}

std::shared_ptr<Gateway> Router::route(const Transaction& /*tx*/) {
    // 1. Roll the dice for Exploration vs Exploitation
    double random_val;
    {
        std::lock_guard<std::mutex> lock(rng_mutex_);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        random_val = dist(rng_);
    }

    if (random_val < epsilon_) {
        // EXPLORE: Pick a completely random gateway
        int random_index;
        {
            std::lock_guard<std::mutex> lock(rng_mutex_);
            std::uniform_int_distribution<int> dist(0, gateways_.size() - 1);
            random_index = dist(rng_);
        }
        return gateways_[random_index];
    } 
    
    // EXPLOIT: Pick the gateway with the highest score
    // We use a shared_lock for READING stats, allowing high concurrency!
    std::shared_lock<std::shared_mutex> read_lock(stats_mutex_);
    
    std::shared_ptr<Gateway> best_gateway = gateways_[0];
    double best_score = -10000.0; // Start with a very low score
    
    for (const auto& gateway : gateways_) {
        std::string id = gateway->get_id();
        double historical_rate = stats_[id].get_success_rate();
        
        // This is your Thundering Herd penalty algorithm!
        int active_connections = gateway->get_active_requests();
        double current_score = historical_rate - (penalty_constant_ * active_connections);
        
        if (current_score > best_score) {
            best_score = current_score;
            best_gateway = gateway;
        }
    }
    
    return best_gateway;
}

void Router::print_stats() const {
    std::shared_lock<std::shared_mutex> lock(stats_mutex_);
    std::cout << "\n--- Router State ---" << std::endl;
    for (const auto& gateway : gateways_) {
        std::string id = gateway->get_id();
        auto it = stats_.find(id);
        GatewayStats stat = (it != stats_.end()) ? it->second : GatewayStats{};
        
        std::cout << id << " | Req: " << stat.total_requests 
                  << " | Succ: " << stat.successful_requests 
                  << " | Rate: " << stat.get_success_rate() 
                  << " | Active: " << gateway->get_active_requests() << std::endl;
    }
    std::cout << "--------------------\n" << std::endl;
}

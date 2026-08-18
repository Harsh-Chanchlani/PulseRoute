#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include "Gateway.hpp"
#include "Router.hpp"
#include "ThreadPool.hpp"
#include "Transaction.hpp"

int main() {
    std::cout << "--- PulseRoute: Adaptive Concurrent Router ---\n" << std::endl;

    // 1. Setup Gateways
    // HDFC is very reliable (99%) but has low capacity (can handle 5 concurrent requests)
    auto hdfc = std::make_shared<Gateway>("HDFC_Gateway", 0.99, 5);
    
    // SBI is less reliable (85%) but can handle more load (15 concurrent requests)
    auto sbi = std::make_shared<Gateway>("SBI_Gateway", 0.85, 15);
    
    // ICICI is a backup (60%) with massive capacity (50 concurrent requests)
    auto icici = std::make_shared<Gateway>("ICICI_Gateway", 0.60, 50);

    std::vector<std::shared_ptr<Gateway>> gateways = {hdfc, sbi, icici};

    // 2. Setup Router 
    // Epsilon = 0.1 (10% exploration)
    // Penalty Constant = 0.05 (Each active connection drops the score by 0.05)
    auto router = std::make_shared<Router>(gateways, 0.1, 0.05);

    // 3. Setup Thread Pool (20 worker threads to generate high concurrent load)
    // Because we have 20 threads and HDFC's capacity is only 5, the Thundering Herd
    // mitigation will definitely trigger and force load balancing!
    ThreadPool pool(20);

    // 4. Generate Load
    int total_transactions = 3000;
    std::cout << "Blasting " << total_transactions << " transactions into the ThreadPool...\n" << std::endl;
    
    for (int i = 1; i <= total_transactions; ++i) {
        
        // At 1/3 mark, simulate a catastrophic failure at HDFC
        if (i == 1000) {
            pool.enqueue([hdfc, router]() {
                std::cout << "\n[ALERT] HDFC Server Outage Simulated! Success rate dropping to 10%\n" << std::endl;
                hdfc->set_success_rate(0.10);
                router->print_stats();
            });
        }

        // At 2/3 mark, simulate HDFC recovering!
        if (i == 2000) {
            pool.enqueue([hdfc, router]() {
                std::cout << "\n[ALERT] HDFC Server Recovered! Success rate restored to 99%\n" << std::endl;
                hdfc->set_success_rate(0.99);
                router->print_stats();
            });
        }

        // Create the transaction
        Transaction tx;
        tx.transaction_id = "TXN_" + std::to_string(i);
        tx.amount = 100.0;
        tx.timestamp = std::chrono::system_clock::now();

        // Enqueue the job for the worker threads
        pool.enqueue([tx, router]() {
            // A. Routing Decision (Thread-safe read)
            auto chosen_gateway = router->route(tx);
            
            // B. Processing (Simulates network latency and capacity checks)
            bool success = chosen_gateway->process(tx);
            
            // C. Feedback Loop (Thread-safe write)
            router->update_stats(chosen_gateway->get_id(), success);
        });
        
        // A very tiny delay to space out the ingestion of traffic slightly
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Give the worker threads time to finish processing the queue
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n--- Final Results ---" << std::endl;
    router->print_stats();

    return 0;
}

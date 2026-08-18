#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t num_threads) : stop_(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                // The thread blocks here until a task is available
                std::function<void()> task = tasks_.pop();
                
                // If we are shutting down and receive an empty task (a "poison pill"), we exit
                if (stop_ && !task) {
                    break;
                }
                
                // Otherwise, if the task is valid, execute it
                if (task) {
                    task(); 
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    stop_ = true;
    
    // Wake up all threads by pushing empty tasks ("poison pills")
    for (size_t i = 0; i < workers_.size(); ++i) {
        tasks_.push(nullptr); 
    }
    
    // Wait for all threads to finish their current task and exit
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    if (!stop_) {
        tasks_.push(std::move(task));
    }
}

#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include "ThreadSafeQueue.hpp"

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    // Adds a new job to the queue
    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers_;
    ThreadSafeQueue<std::function<void()>> tasks_;
    std::atomic<bool> stop_;
};

#pragma once

#include <string>
#include <chrono>

struct Transaction {
    std::string transaction_id;
    double amount;
    std::chrono::system_clock::time_point timestamp;
};

# PulseRoute: Adaptive Concurrent Payment Router
## Project Specification & Architecture Document

**Target Audience:** AI Coding Agent / Developer Assistant (e.g., Antigravity, Cursor, Copilot)
**Goal:** Build a concurrent, adaptive payment-gateway router in modern C++ that mimics enterprise-level payment orchestration (like Juspay's routing engine).
**Primary Focus Areas:** Multithreading, OS-level synchronization (Condition Variables, Mutexes), Data Structures, and Adaptive Algorithms.

---

## 1. System Overview
PulseRoute simulates a high-throughput transaction router. It ingests thousands of concurrent payment requests, queues them, and utilizes a thread pool of worker threads to process them. Instead of routing requests randomly or via round-robin, the system uses a simple Epsilon-Greedy algorithm to dynamically route transactions to the most reliable mock payment gateways in real-time.

## 2. Technical Stack & Core Requirements
*   **Language:** Modern C++ (C++17 or C++20).
*   **Concurrency:** Standard Threading Library (`<thread>`, `<mutex>`, `<condition_variable>`, `<atomic>`).
*   **Data Structures:** `std::queue` (for the task buffer), `std::vector` (for gateway management).
*   **No External Dependencies:** Strictly use the C++ Standard Library.

---

## 3. Core Architectural Components

### A. The Ingestion Layer (Producer)
*   **Role:** Simulates incoming payment traffic spikes.
*   **Implementation:** A producer thread that generates `Transaction` structs (containing `transaction_id`, `timestamp`, and `amount`) and pushes them into a shared buffer. 

### B. The Concurrency Engine (Thread Pool & Task Queue)
*   **Role:** Manages the concurrent processing of transactions without creating/destroying threads constantly.
*   **Implementation:** 
    *   A thread-safe `std::queue<Transaction>`.
    *   A Thread Pool with a fixed number of worker threads (Consumers).
    *   **Synchronization:** Uses `std::mutex` and `std::condition_variable`. Worker threads sleep when the queue is empty and are awakened via `cv.notify_one()` or `cv.notify_all()` when the Producer pushes a new transaction.

### C. The Gateway Simulators (Destinations)
*   **Role:** Mock payment gateways (e.g., HDFC, SBI, ICICI) that process transactions.
*   **Implementation:** Objects that receive a transaction and return a boolean success/failure.
    *   *Crucial Feature:* They must have dynamic, fluctuating failure rates and latency. For instance, a gateway might start with a 99% success rate but drop to 40% to simulate a temporary server outage.

### D. The Adaptive Routing Engine (The Brain)
*   **Role:** Decides which gateway should process a given transaction.
*   **Implementation:** Uses a simple **Epsilon-Greedy algorithm**.
    *   **Exploration (e.g., 10%):** Routes to a random gateway to discover if previously failing gateways have recovered.
    *   **Exploitation (e.g., 90%):** Routes to the gateway with the current highest running success rate.
    *   *Note:* Uses "Optimistic Initialization" (assuming a 1.0 success rate for untested gateways) to ensure all gateways receive initial traffic.

### E. The State Logger (Write-back Cache Simulation)
*   **Role:** Thread-safely logs the outcome of transactions.
*   **Implementation:** Worker threads record successes/failures. Periodically, this state is flushed/printed to the console to visualize the router's adaptive behavior.

---

## 4. Execution Flow (Lifecycle of a Transaction)

1.  **Generation:** The Producer thread creates a `Transaction` and locks the mutex to push it into the `Task Queue`.
2.  **Notification:** The Producer signals the `condition_variable`, waking up an idle Worker thread in the Thread Pool.
3.  **Routing Decision:** The Worker thread pops the `Transaction`, unlocks the queue, and queries the `Routing Engine`.
4.  **Algorithmic Selection:** The Routing Engine applies the simple Epsilon-Greedy logic and returns the ID of the chosen `Gateway Simulator`.
5.  **Processing & Result:** The Worker thread sends the transaction to the chosen Gateway. The Gateway simulates network latency and returns a success/fail boolean based on its current internal state.
6.  **State Update:** The Worker thread thread-safely updates the `GatewayStats` (total requests vs. successful requests) for the chosen gateway, allowing the routing engine to adapt based on the result.

---

## 5. Agent Instructions for Implementation Phases

When writing the code, please proceed in the following modular phases:

*   **Phase 1: Concurrency Foundation.** Implement the thread-safe queue, mutexes, condition variables, and a basic Thread Pool. Test with dummy string tasks.
*   **Phase 2: Gateways & Transactions.** Create the `Transaction` struct and the `Gateway` classes with randomized, fluctuating success rates.
*   **Phase 3: The Adaptive Routing Layer.** Implement the simple Epsilon-Greedy router algorithm and the thread-safe state tracking for gateway success rates.
*   **Phase 4: Integration & Stress Test.** Wire everything together. Implement a main function that blasts 10,000+ transactions through the system and outputs a summary showing how the router adapted to failing gateways over time. Ensure code is highly optimized, thread-safe, and free of race conditions.
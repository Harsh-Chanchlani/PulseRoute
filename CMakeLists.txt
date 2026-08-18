cmake_minimum_required(VERSION 3.10)

# Project Name and Version
project(PulseRoute VERSION 1.0 DESCRIPTION "Adaptive Concurrent Payment Router")

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tell the compiler where to find our header files (.hpp)
include_directories(include)

# Create the executable from our source files
add_executable(PulseRoute 
    src/main.cpp 
    src/Gateway.cpp 
    src/Router.cpp 
    src/ThreadPool.cpp
)

# Enable strict warnings to catch bugs early
target_compile_options(PulseRoute PRIVATE -Wall -Wextra)

# Link the standard threading library (required for Linux/macOS)
find_package(Threads REQUIRED)
target_link_libraries(PulseRoute PRIVATE Threads::Threads)

cmake_minimum_required(VERSION 3.21)
project(DianaThreadPoolTest)

include_directories(../ThreadPool)
include_directories(../MultiplePool)
include_directories(../Utilities)

add_executable(DianaThreadPoolTest
        main.cpp
        benchmark.hpp
        ../ThreadPool/ThreadPool.hpp
        ../ThreadPool/ThreadPoolWithSafeQueue.hpp
        ../MultiplePool/MultiplePool.hpp
        ../Utilities/SafeQueue.hpp
        ../Utilities/ScopedTimer.hpp)
#ifndef DIANATHREADPOOL_BENCHMARK_HPP
#define DIANATHREADPOOL_BENCHMARK_HPP

#include <atomic>
#include <chrono>
#include <iostream>

#include "MultiplePool.hpp"
#include "ScopedTimer.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolWithSafeQueue.hpp"

const size_t COUNT = 1'000'000;
const size_t REPS = 10;

void benchmark() {
	std::atomic<int> count = 0;
	Diana::ScopedTimer timer("Simple Thread Pool");
	{
		Diana::ThreadPool pool;
		for (size_t i = 0; i < COUNT; ++i) {
			pool.enqueue([i, &count] {
				count++;
				int x = 0;
				auto reps = REPS + (REPS + (rand() % 5));
				for (size_t j = 0; j < reps; j++) {
					x = i + rand();
				}
			});
		}
	}
	assert(count == COUNT);
}

void benchmark1() {
	std::atomic<int> count = 0;
	Diana::ScopedTimer timer("Simple Thread Pool With Safe Queue");
	{
		Diana::SimplePool pool;
		for (size_t i = 0; i < COUNT; ++i) {
			pool.enqueue([i, &count] {
				count++;
				int x = 0;
				auto reps = REPS + (REPS + (rand() % 5));
				for (size_t j = 0; j < reps; j++) {
					x = i + rand();
				}
			});
		}
	}
	assert(count == COUNT);
}

void benchmark2() {
	std::atomic<int> count = 0;
	Diana::ScopedTimer timer("MultiplePool");
	{
		Diana::MultiplePool pool;
		for (size_t i = 0; i < COUNT; ++i) {
			pool.schedule_by_id([i, &count] {
				count++;
				int x = 0;
				auto reps = REPS + (REPS + (rand() % 5));
				for (size_t j = 0; j < reps; j++) {
					x = i + rand();
				}
			});
		}
	}
	assert(count == COUNT);
}


#endif//DIANATHREADPOOL_BENCHMARK_HPP
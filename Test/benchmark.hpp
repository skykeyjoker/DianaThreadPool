#ifndef DIANATHREADPOOL_BENCHMARK_HPP
#define DIANATHREADPOOL_BENCHMARK_HPP

#include <atomic>
#include <chrono>
#include <iostream>

#include "MultiplePool.hpp"
#include "ScopedTimer.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolWithSafeQueue.hpp"
#include "Worksteal.hpp"

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

void autoScheduleTasks(bool enableWorkSteal) {
	std::atomic<int> count = 0;
	{
		Diana::WorkStealThreadPool tp(std::thread::hardware_concurrency(), enableWorkSteal);

		for (int i = 0; i < COUNT; ++i) {
			[[maybe_unused]] auto ret = tp.scheduleById([i, &count] {
				count++;
				int x;
				auto reps = REPS + (REPS * (rand() % 5));
				for (int n = 0; n < reps; ++n)
					x = i + rand();
				(void) x;
			});
			assert(ret == Diana::WorkStealThreadPool::ERROR_TYPE::ERROR_NONE);
		}
	}
	assert(count == COUNT);
}

void benchmark3() {
	Diana::ScopedTimer timer1("ThreadPool_noWorkSteal");
	{
		autoScheduleTasks(false);
	}

	Diana::ScopedTimer timer2("ThreadPool_withWorkSteal");
	{
		autoScheduleTasks(false);
	}
}


#endif//DIANATHREADPOOL_BENCHMARK_HPP
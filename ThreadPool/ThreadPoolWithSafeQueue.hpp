#ifndef DIANATHREADPOOL_THREADPOOLWITHSAFEQUEUE_H
#define DIANATHREADPOOL_THREADPOOLWITHSAFEQUEUE_H

#include <functional>
#include <thread>
#include <vector>

#include "../Utilities/SafeQueue.hpp"

namespace Diana {

	using WorkItem = std::function<void()>;
	// * 简易多线程单任务队列线程池，使用SafeQueue线程安全队列。
	class SimplePool {
	public:
		explicit SimplePool(size_t threads = std::thread::hardware_concurrency()) {
			for (size_t i = 0; i < threads; ++i) {
				workers_.emplace_back([this]() {
					for (;;) {
						std::function<void()> task;
						if (!queue_.pop(task))
							return;

						if (task)
							task();
					}
				});
			}
		}

		void enqueue(WorkItem item) {
			queue_.push(std::move(item));
		}

		~SimplePool() {
			queue_.stop();
			for (auto& thd: workers_)
				thd.join();
		}

	private:
		SafeQueue<WorkItem> queue_;
		std::vector<std::thread> workers_;
	};

};// namespace Diana

#endif//DIANATHREADPOOL_THREADPOOLWITHSAFEQUEUE_H

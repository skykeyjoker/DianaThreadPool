#ifndef DIANATHREADPOOL_MULTIPLEPOOL_HPP
#define DIANATHREADPOOL_MULTIPLEPOOL_HPP

#include <cassert>
#include <functional>
#include <thread>
#include <vector>

#include "../Utilities/SafeQueue.hpp"

namespace Diana {

	using WorkItem = std::function<void()>;
	// * 简易多线程多任务队列线程池，使用SafeQueue线程安全队列。
	class MultiplePool {
	public:
		explicit MultiplePool(size_t thread_num = std::thread::hardware_concurrency())
			: queues_(thread_num),
			  thread_num_(thread_num) {
			auto worker = [this](size_t id) {
				while (true) {
					WorkItem task{};
					if (!queues_[id].pop(task))
						break;

					if (task)
						task();
				}
			};

			workers_.reserve(thread_num_);
			for (size_t i = 0; i < thread_num_; ++i) {
				workers_.emplace_back(worker, i);
			}
		}

		int schedule_by_id(WorkItem fn, size_t id = -1) {
			if (fn == nullptr)
				return -1;

			if (id == -1) {
				id = rand() % thread_num_;// 随机插入到一个线程的任务队列中
				queues_[id].push(std::move(fn));
			} else {
				assert(id < thread_num_);// 插入指定线程的任务队列
				queues_[id].push(std::move(fn));
			}

			return 0;
		}

		~MultiplePool() {
			for (auto& queue: queues_) {
				queue.stop();// 停止每一个任务队列
			}
			for (auto& worker: workers_) {
				worker.join();// 阻塞，等待每个线程执行结束
			}
		}

	private:
		std::vector<Diana::SafeQueue<WorkItem>> queues_;// 每个线程对应一个任务队列
		size_t thread_num_;
		std::vector<std::thread> workers_;
	};

};// namespace Diana

#endif//DIANATHREADPOOL_MULTIPLEPOOL_HPP

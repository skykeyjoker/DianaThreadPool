#ifndef DIANATHREADPOOL_WORKSTEAL_HPP
#define DIANATHREADPOOL_WORKSTEAL_HPP

#include <atomic>
#include <cassert>
#include <functional>
#include <thread>
#include <vector>

#include "../Utilities/SafeQueue.hpp"

namespace Diana {
	class WorkStealThreadPool {
	public:
		struct WorkItem {
			bool canSteal = false;// 是否支持work steal
			std::function<void()> fn = nullptr;
		};

		enum ERROR_TYPE {
			ERROR_NONE = 0,
			ERROR_POOL_HAS_STOP,
			ERROR_ITEM_IS_NULL
		};

		explicit WorkStealThreadPool(size_t threadNum = std::thread::hardware_concurrency(),
									 bool enableWorkSteal = false);

		~WorkStealThreadPool();

		WorkStealThreadPool::ERROR_TYPE scheduleById(std::function<void()> fn,
													 int32_t id = -1);

		int32_t getCurrentId() const;
		size_t getItemCount() const;
		int32_t getThreadNum() const { return m_threadNum; }

	private:
		std::pair<size_t, WorkStealThreadPool *> *getCurrent() const;
		int32_t m_threadNum;
		std::vector<SafeQueue<WorkItem>> m_queues;
		std::vector<std::thread> m_threads;
		bool m_enableWorkSteal;
		std::atomic<bool> m_stop;
	};

	inline WorkStealThreadPool::WorkStealThreadPool(size_t threadNum, bool enableWorkSteal)
		: m_threadNum(threadNum ? threadNum : std::thread::hardware_concurrency()),
		  m_queues(m_threadNum),
		  m_enableWorkSteal(enableWorkSteal),
		  m_stop(false) {
		auto worker = [this](size_t id) {
			auto current = getCurrent();
			current->first = id;
			current->second = this;
			while (true) {
				WorkItem workerItem = {};
				if (m_enableWorkSteal) {
					// 尝试work steal
					for (int n = 0; n < m_threadNum * 2; ++n) {
						if (m_queues[(id + n) % m_threadNum].try_pop_if(
									workerItem,
									[](auto &item) { return item.canSteal; }))
							break;
					}
				}

				// Work Steal机制关闭或Work Steal失败，阻塞获取任务
				if (!workerItem.fn && !m_queues[id].pop(workerItem)) {
					break;
				}

				if (workerItem.fn) {
					workerItem.fn();
				}
			}
		};

		m_threads.reserve(m_threadNum);
		for (auto i = 0; i < m_threadNum; ++i) {
			m_threads.emplace_back(worker, i);
		}
	}

	inline WorkStealThreadPool::~WorkStealThreadPool() {
		m_stop = true;
		for (auto &queue: m_queues)
			queue.stop();
		for (auto &thread: m_threads)
			thread.join();
	}

	inline WorkStealThreadPool::ERROR_TYPE WorkStealThreadPool::scheduleById(std::function<void()> fn, int32_t id) {
		if (fn == nullptr)
			return ERROR_ITEM_IS_NULL;

		if (m_stop)
			return ERROR_POOL_HAS_STOP;

		if (id == -1) {// 随机分配任务至任意线程的工作队列
			if (m_enableWorkSteal) {
				// 尝试非阻塞方式提交到任务队列
				WorkItem workerItem{.canSteal = true, .fn = fn};// C++20
				for (int n = 0; n < m_threadNum * 2; ++n) {
					if (m_queues.at(n % m_threadNum).try_push(workerItem))
						return ERROR_NONE;
				}
			}

			id = rand() % m_threadNum;
			m_queues[id].push(
					WorkItem{.canSteal = m_enableWorkSteal, .fn = std::move(fn)});
		} else {
			assert(id < m_threadNum);
			m_queues[id].push(WorkItem{.canSteal = false, .fn = std::move(fn)});
		}

		return ERROR_NONE;
	}

	inline std::pair<size_t, WorkStealThreadPool *> *WorkStealThreadPool::getCurrent() const {
		static thread_local std::pair<size_t, WorkStealThreadPool *> current(-1, nullptr);
		return &current;
	}

	inline int32_t WorkStealThreadPool::getCurrentId() const {
		auto current = getCurrent();
		if (this == current->second)
			return current->first;
		return -1;
	}

	inline size_t WorkStealThreadPool::getItemCount() const {
		size_t ret{0};
		for (int i = 0; i < m_threadNum; ++i) {
			ret += m_queues[i].size();
		}
		return ret;
	}

}// namespace Diana

#endif//DIANATHREADPOOL_WORKSTEAL_HPP

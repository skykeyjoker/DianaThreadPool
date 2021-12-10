#ifndef DIANATHREADPOOL_SAFEQUEUE_H
#define DIANATHREADPOOL_SAFEQUEUE_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

namespace Diana {

	// * 线程安全队列
	template<typename T>
	class SafeQueue {
	public:
		void push(const T &item) {
			{
				std::scoped_lock lock(mtx_);
				queue_.push(item);
			}
			cond_.notify_one();
		}

		void push(T &&item) {// 两个push方法，此处不是万能引用而是单纯右值
			{
				std::scoped_lock lock(mtx_);
				queue_.push(std::move(item));
			}
			cond_.notify_one();
		}

		bool pop(T &item) {
			std::unique_lock lock(mtx_);
			cond_.wait(lock, [&]() {
				return !queue_.empty() || stop_;
			});
			if (queue_.empty())
				return false;
			item = std::move(queue_.front());
			queue_.pop();
			return true;
		}

		std::size_t size() const {
			std::scoped_lock lock(mtx_);
			return queue_.size();
		}

		bool empty() const {
			std::scoped_lock lock(mtx_);
			return queue_.empty();
		}

		void stop() {
			{
				std::scoped_lock lock(mtx_);
				stop_ = true;
			}
			cond_.notify_all();
		}

	private:
		std::condition_variable cond_;
		mutable std::mutex mtx_;
		std::queue<T> queue_;
		bool stop_ = false;
	};

};// namespace Diana

#endif//DIANATHREADPOOL_SAFEQUEUE_H

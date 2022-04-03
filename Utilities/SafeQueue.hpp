#ifndef DIANATHREADPOOL_SAFEQUEUE_H
#define DIANATHREADPOOL_SAFEQUEUE_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <utility>

namespace Diana {

	// * 线程安全队列
	template<typename T>
	requires std::is_move_assignable_v<T>// Concept (C++20) T类型为可移动赋值
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

		bool try_push(const T &item) {// 非阻塞方式入队列
			{
				std::unique_lock lock(mtx_, std::try_to_lock);
				if (!lock)
					return false;
				queue_.push(item);
			}
			cond_.notify_one();
			return true;
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

		bool try_pop(T &item) {                           // 非阻塞方式获取队列元素
			std::unique_lock lock(mtx_, std::try_to_lock);// 非阻塞的方式获取一把锁
			if (!lock || queue_.empty())
				return false;// 获取锁失败或队列为空
			item = std::move(queue_.front());
			queue_.pop();
			return true;
		}

		bool try_pop_if(T &item, bool (*predict)(T &) = nullptr) {// 条件非阻塞方式获取锁
			std::unique_lock lock(mtx_, std::try_to_lock);
			if (!lock || queue_.empty())
				return false;

			if (predict && !predict(queue_.front()))
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

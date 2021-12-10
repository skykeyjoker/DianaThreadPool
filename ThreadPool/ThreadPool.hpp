#ifndef DIANATHREADPOOL_THREADPOOL_HPP
#define DIANATHREADPOOL_THREADPOOL_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Diana {

	// * 简易多线程单任务队列线程池，使用线程安全队列，接口更人性化。
	class ThreadPool {
	public:
		explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) : stop(false) {
			// 根据threads数量创建多个线程
			for (size_t i = 0; i < threads; ++i) {
				workers.emplace_back([this]() {
					for (;;) {// 工作线程就是一个死循环，不停查询任务队列并取出任务执行
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->queue_mutex);
							this->condition.wait(lock,
												 [this]() {
													 return this->stop || !this->tasks.empty();// 条件变量等待线程池不为空或者stop
												 });
							if (this->stop && this->tasks.empty())// 线程池为空且stop，证明线程池结束，退出线程
								return;
							task = std::move(this->tasks.front());// 取出任务
							this->tasks.pop();
						}

						task();// 执行任务
					}
				});// lambda表达式构建
			}
		}

		template<typename F, typename... Args>
		auto enqueue(F &&f, Args &&...args) {
			using return_type = std::invoke_result_t<F, Args...>;
			auto task = std::make_shared<std::packaged_task<return_type()>>(
					std::bind(std::forward<F>(f), std::forward<Args>(args)...));// 完美转发，构造任务仿函数的指针
			std::future<return_type> res = task->get_future();                  // 获得函数执行的future返回
			{
				std::unique_lock<std::mutex> lock(queue_mutex);

				if (stop) {
					throw std::runtime_error("enqueue on stopped Thread pool");
				}

				tasks.emplace([task = std::move(task)]() { (*task)(); });// 塞入任务队列
			}                                                            // 入队列后即可解锁
			condition.notify_one();                                      // 仅唤醒一个线程，避免无意义的竞争
			return res;
		}

		~ThreadPool() {
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				stop = true;
			}
			condition.notify_all();// 唤醒所有线程，清理任务
			for (std::thread &worker: workers)
				worker.join();// 阻塞，等待所有线程执行结束
		}

	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};

};// namespace Diana


#endif//DIANATHREADPOOL_THREADPOOL_HPP

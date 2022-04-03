#include <iostream>
#include <string>

#include "MultiplePool.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolWithSafeQueue.hpp"
#include "Worksteal.hpp"
#include "benchmark.hpp"

void test_thread_pool() {
	std::cout << "test_thread_pool()" << std::endl;
	Diana::ThreadPool threadPool;
	threadPool.enqueue([] { std::cout << "hello\n"; });
	auto future = threadPool.enqueue([](std::string str) { return "hello" + str; }, "world");
	std::cout << future.get() << std::endl;
}

std::string funA(std::string str) {
	return "hello" + str;
}

void test_simple_thread_pool() {
	std::cout << "test_simple_thread_pool()" << std::endl;
	Diana::SimplePool threadPool;
	threadPool.enqueue([] { std::cout << "hello\n"; });
	// * 此处必须使用shared_ptr进行包装，
	// * 否则在function<void()>中会尝试生成std::packaged_task的拷贝构造函数，
	// ! std::packaged_task禁止拷贝操作
	auto task = std::make_shared<std::packaged_task<std::string()>>(std::bind(funA, "world"));
	std::future<std::string> res = task->get_future();
	threadPool.enqueue([task = std::move(task)] { (*task)(); });
	//	auto task = std::packaged_task<std::string()>(std::bind(funA, "world"));
	//	std::future<std::string> res = task.get_future();
	//	threadPool.enqueue(std::move(task));
	std::cout << res.get() << std::endl;
}

void test_multiple_thread_pool() {
	std::cout << "test_multiple_thread_pool()" << std::endl;
	Diana::MultiplePool threadPool;
	threadPool.schedule_by_id([] { std::cout << "hello\n"; });
	auto task = std::make_shared<std::packaged_task<std::string()>>(std::bind(funA, "world"));
	std::future<std::string> res = task->get_future();
	threadPool.schedule_by_id([task = std::move(task)] { (*task)(); });
	std::cout << res.get() << std::endl;
}

void test_worksteal_thread_pool() {
	std::cout << "test_worksteal_thread_pool()" << std::endl;
	Diana::WorkStealThreadPool threadPool(16, true);
	threadPool.scheduleById([] { std::cout << "hello\n"; });
	auto task = std::make_shared<std::packaged_task<std::string()>>(std::bind(funA, "world"));
	std::future<std::string> res = task->get_future();
	threadPool.scheduleById([task = std::move(task)] { (*task)(); });
	std::cout << res.get() << std::endl;
}

int main() {
	std::cout << "test begin" << std::endl;

	// test
	test_thread_pool();
	test_simple_thread_pool();
	test_multiple_thread_pool();
	test_worksteal_thread_pool();

	// benchmark
	for (size_t i = 0; i < 6; ++i) {
		benchmark();
		benchmark1();
		benchmark2();
		benchmark3();
	}

	std::cout << "test end" << std::endl;

	return 0;
}

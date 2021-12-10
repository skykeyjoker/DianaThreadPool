#ifndef DIANATHREADPOOL_SCOPEDTIMER_H
#define DIANATHREADPOOL_SCOPEDTIMER_H

#include <chrono>
#include <iostream>

namespace Diana {

	// * RAII式计时器
	class ScopedTimer {
	public:
		ScopedTimer(const char* name)
			: m_name(name),
			  m_beg(std::chrono::high_resolution_clock::now()) {}

		~ScopedTimer() {
			auto end = std::chrono::high_resolution_clock::now();
			auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_beg);
			std::cout << m_name << " : " << dur.count() << " ns" << std::endl;
		}

	private:
		const char* m_name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
	};

};// namespace Diana

#endif//DIANATHREADPOOL_SCOPEDTIMER_H

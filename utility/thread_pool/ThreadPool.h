#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstdint>
#include <atomic>

namespace common_app
{

typedef std::function<void(void)> ThreadPoolCallback;

class ThreadPool
{
public:
	ThreadPool();
	explicit ThreadPool(uint32_t thread_count, uint32_t queue_size=512);	//	should be appropriate value
	~ThreadPool();
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void startThreadPool();
	void postCallback(ThreadPoolCallback cb);

private:
	enum
	{
		MAX_THREAD_COUNT = 16,
		MAX_QUEUE_SIZE = 512 * 8,
	};

	void callbackInThread();

	std::atomic<bool> running_;
	uint32_t thread_count_;
	uint32_t queue_size_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::queue<ThreadPoolCallback> callbacks_;
	std::vector<std::thread> threads_;
};

}	//	namespace common_app

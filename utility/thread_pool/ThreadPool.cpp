#include "Common/SDK/utility/thread_pool/ThreadPool.h"

namespace common_app
{

ThreadPool::ThreadPool()
	: running_(false),
	  thread_count_(std::thread::hardware_concurrency()),
	  queue_size_(512)
{
}

ThreadPool::ThreadPool(uint32_t thread_count, uint32_t queue_size/*=512*/)
	: running_(false),
	  thread_count_(thread_count),
	  queue_size_(queue_size)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
		running_ = false;
	cv_.notify_all();
	for (auto&item : threads_)
		item.join();
}

void ThreadPool::startThreadPool()
{
	if (running_)
		return;
	running_ = true;
	if (thread_count_ <= 0 || thread_count_ > MAX_THREAD_COUNT)
		thread_count_ = 1;
	if (queue_size_ <= 0 || queue_size_ > MAX_QUEUE_SIZE)
		queue_size_ = 512;
	for (auto i = 0; i < thread_count_; ++i)
	{
		threads_.emplace_back(std::thread(std::bind(&ThreadPool::callbackInThread, this)));
	}
}

void ThreadPool::postCallback(ThreadPoolCallback cb)
{
	std::unique_lock<std::mutex> ulck(mutex_);
	while (callbacks_.size() >= queue_size_)
		callbacks_.pop();
	callbacks_.push(std::move(cb));
	cv_.notify_one();
}

void ThreadPool::callbackInThread()
{
	while (running_)
	{
		ThreadPoolCallback callback;
		{
			std::unique_lock<std::mutex> ulck(mutex_);
			cv_.wait(ulck,
				[this]() -> bool
			{
				return !running_ || !callbacks_.empty();
			});
			if (!running_)
				break;
			if (!callbacks_.empty())
			{
				callback = callbacks_.front();
				callbacks_.pop();
			}
		}
		if (callback)
		{
			try
			{
				callback();
			}
			catch (...)
			{
			}
		}
	}
}

}	//	namespace common_app

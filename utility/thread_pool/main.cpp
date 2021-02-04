#include "ThreadPool.h"
#include <map>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace common_app;

void map_test()
{
	std::map<std::string, std::string> map_test;
	map_test[std::string()] = std::string("empty");
	map_test[std::string("0")] = std::string("0");
	map_test[std::string("1")] = std::string("1");
	for (const auto&item : map_test)
	{
		std::cout << item.first << "," << item.second << std::endl;
	}
	std::cout << map_test[""] << std::endl;
}

ThreadPool thread_pool;

void thread_test()
{
	std::cout << "post start" << std::endl;
	thread_pool.startThreadPool();
	for (auto i = 0; i < 100; ++i)
	{
		thread_pool.postCallback(
			[i]()
		{
			std::cout << "count is " << i << ", thread id is " << std::this_thread::get_id() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		});
	}
	std::cout << "post finish" << std::endl;
}

class ArrayDeleter
{
public:
	template<typename T>
	void operator()(T* p) const
	{
		std::cout << "delete pointer which points to array" << std::endl;
		delete[] p;
	}
};

void arraydeleter_test()
{
	std::unique_ptr<int, ArrayDeleter> iarray(new int[16], ArrayDeleter());
	std::cout << sizeof iarray << std::endl;
	std::cout << sizeof iarray.get() << std::endl;
}

int main(int argc, char** argv)
{
	//map_test();
	//thread_test();
	arraydeleter_test();

	getchar();
	return 0;
}

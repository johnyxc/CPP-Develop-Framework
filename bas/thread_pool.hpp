#ifndef __THREAD_POOL_HPP_2021_06_04__
#define __THREAD_POOL_HPP_2021_06_04__
#include <asio/asio.hpp>
#include <bas/osfunc.hpp>
#include <bas/strand.hpp>
#include <map>
#include <queue>
#include <list>
#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <utility/log/Log.h>
using namespace jf_log;
#define DEF_THREAD_COUNT 2

namespace bas
{
	namespace detail
	{
		using standard_call = std::function<void()>;
		enum { MAX_TASK_SIZE = 1000000 };

		struct inner_thread_t
		{
			inner_thread_t() : alive_(true)
			{
				evt_ = get_event_hdl();
			}

			void start(int idx)
			{
				work_trd_ = std::make_unique<std::thread>([this]() {
					while (alive_)
					{
						standard_call fo;
						auto list_size = 0;

						fo_mtx_.lock();
						if (!fo_list_.empty())
						{
							fo = fo_list_.front();
							fo_list_.pop_front();
							list_size = fo_list_.size();
						}
						fo_mtx_.unlock();

						if (fo)
						{
							try
							{
								fo();
							}
							catch (std::exception& e) {
								std::cout << "thread_pool exception : " << e.what() << std::endl;
							}
							catch (...) {
								std::cout << "thread_pool exception" << std::endl;
							}
						}

						if (!list_size)
						{
							event_wait(evt_);
						}
					}
				});
				SetThreadAffinityMask(work_trd_->native_handle(), 1 << idx);
			}

			void stop()
			{
				alive_ = false;
				work_trd_->join();
			}

			void post(standard_call fo)
			{
				fo_mtx_.lock();
				fo_list_.push_back(fo);
				fo_mtx_.unlock();
				set_event(evt_);
			}

			bool is_busy()
			{
				return get_task_size() > MAX_TASK_SIZE;
			}

			size_t get_task_size()
			{
				auto list_size = 0;

				fo_mtx_.lock();
				list_size = fo_list_.size();
				fo_mtx_.unlock();

				return list_size;
			}

			std::list<standard_call>		fo_list_;
			std::unique_ptr<std::thread>	work_trd_;
			std::mutex						fo_mtx_;
			HEVENT							evt_;
			bool							alive_;
		};

		struct thread_pool_t
		{
		public:
			thread_pool_t() : worker_count_()
			{}
			~thread_pool_t()
			{}

		public:
			void start()
			{
				worker_count_ = std::thread::hardware_concurrency();
				if (!worker_count_) worker_count_ = DEF_THREAD_COUNT;
				worker_ = std::make_unique<asio::io_service::work>(io_);

				for (int i = 0; i < worker_count_; i++)
				{
					auto trd = std::make_shared<std::thread>(std::bind(&thread_pool_t::i_on_thread, this));
					SetThreadAffinityMask(trd->native_handle(), 1 << i);
					threads_.emplace_back(trd);
				}

				for (int i = 0; i < worker_count_; i++)
				{
					auto worker = std::make_shared<inner_thread_t>();
					workers_.emplace_back(worker);
					worker->start(i);
				}
			}

			void stop()
			{
				worker_.reset();
				io_.stop();

				for (auto& item : workers_)
				{
					item->stop();
				}
				workers_.clear();
			}

			//	通过此函数投递的函数对象需要提供 strand 参数
			//	同一个 strand 下的函数对象保证串行执行
			bool post_call(standard_call fo, std::shared_ptr<strand_t> strand)
			{
				auto post_ok = true;

				mtx_.lock();
				auto iter = strand_proc_list_.find(strand.get());
				if (iter != strand_proc_list_.end())
				{
					if (iter->second->is_busy()) {
						post_ok = false;
					} else {
						iter->second->post(fo);
					}
				}
				else
				{
					strand_proc_list_[strand.get()] = i_get_appropriate_worker();
					strand_proc_list_[strand.get()]->post(fo);
				}
				mtx_.unlock();

				return post_ok;
			}

			asio::io_service& get_service() { return io_; }

		private:
			void i_on_thread()
			{
				try
				{
					io_.run();
				}
				catch (...) {}
			}

			std::shared_ptr<inner_thread_t> i_get_appropriate_worker()
			{
				unsigned long long smallest = UINT64_MAX;
				std::shared_ptr<inner_thread_t> worker;

				for (auto i = 0; i < worker_count_; i++)
				{
					auto task_size = workers_[i]->get_task_size();
					if (task_size <= smallest)
					{
						smallest = task_size;
						worker = workers_[i];
					}
				}

				return worker;
			}

		public:
			static thread_pool_t* instance()
			{
				static thread_pool_t self;
				return &self;
			}

		private:
			int													 worker_count_;

			asio::io_service									 io_;
			std::unique_ptr<asio::io_service::work>				 worker_;
			std::vector<std::shared_ptr<std::thread>>			 threads_;

			std::vector<std::shared_ptr<inner_thread_t>>		 workers_;
			std::map<strand_t*, std::shared_ptr<inner_thread_t>> strand_proc_list_;
			std::mutex											 mtx_;
		};
	}

	static detail::thread_pool_t* default_thread_pool() { return detail::thread_pool_t::instance(); }
}

#endif

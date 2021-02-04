#ifndef __THREAD_POOL_HPP_2016_09_19__
#define __THREAD_POOL_HPP_2016_09_19__
#include <asio/asio.hpp>
#include <bas/osfunc.hpp>
#include <bas/strand.hpp>
#include <map>
#include <queue>
#include <list>
#include <vector>
#include <memory>
#include <thread>
#include <utility/log/Log.h>
using namespace jf_log;
#define	MAX_THREAD_COUNT	4
#define INNER_THREAD_COUNT	2
//static int g_core_count = std::thread::hardware_concurrency();

namespace bas
{
	namespace detail
	{
		struct thread_pool_t
		{
			using standard_call = std::function<void()>;

			struct pt_param
			{
				pt_param() : alive(false) {}
				std::unique_ptr<std::thread> trd;
				HEVENT evt;
				bool alive;
			};

			//	投递事件结构
			struct exe_param
			{
				std::function<void ()> fo;
				std::shared_ptr<strand_t> strand;
				long long time;
			};

			struct exe_cmp
			{
				bool operator () (const exe_param* a, const exe_param* b) { return a->time > b->time; }
			};

		public :
			thread_pool_t() : tc_(), io_(MAX_THREAD_COUNT)
			{
				evt_			= get_event_hdl(false, false);
				list_mutex_		= get_mutex();
				own_list_mutex_ = get_mutex();
			}
			~thread_pool_t()
			{
				release_event_hdl(evt_);
				release_mutex(list_mutex_);
				release_mutex(own_list_mutex_);
			}

		public :
			void start()
			{
				//	开启 asio 线程池
				tc_ = MAX_THREAD_COUNT;
				worker_ = std::make_unique<asio::io_service::work>(io_);
				for (int i = 0; i < tc_; i++)
				{
					threads_[i] = std::make_unique<std::thread>(
						std::bind(&thread_pool_t::i_on_thread, this)
						);
				}

				//	开启内部线程池
				build_thread_alive_ = true;
				build_thread_ = std::make_unique<std::thread>(
					std::bind(&thread_pool_t::i_on_thread_build, this));

				for (int i = 0; i < INNER_THREAD_COUNT; i++)
				{
					post_thread_[i].evt = get_event_hdl(false, false);
					post_thread_[i].alive = true;
					post_thread_[i].trd = std::make_unique<std::thread>(
						std::bind(&thread_pool_t::i_on_thread_get, this, &post_thread_[i]));
				}
			}

			void stop()
			{
				worker_.reset();
				io_.stop();

				build_thread_alive_ = false;
				set_event(evt_);

				for (int i = 0; i < tc_; i++)
				{
					threads_[i]->join();
				}

				for (int i = 0; i < INNER_THREAD_COUNT; i++)
				{
					post_thread_[i].alive = false;
					set_event(post_thread_[i].evt);
					post_thread_[i].trd->join();
				}

				build_thread_->join();
			}

			asio::io_service& get_service()
			{
				return io_;
			}

			//	暂不提供
			void post(standard_call fo) {}

			//	通过此函数投递的函数对象需要提供 strand 参数
			//	同一个 strand 下的函数对象保证串行独占执行
			void post_call(standard_call fo, std::shared_ptr<strand_t> strand)
			{
				if (!strand) return;

				exe_param* ep = new exe_param;
				ep->fo = fo;
				ep->strand = strand;
				ep->time = GetTickCount();

				lock(own_list_mutex_);
				auto iter = own_event_list_.find(strand.get());
				if (iter == own_event_list_.end())
				{
					std::list<exe_param*> exe_list;
					exe_list.push_back(ep);
					own_event_list_.insert(std::pair<strand_t*, std::list<exe_param*>>(strand.get(), exe_list));
				}
				else
				{
					iter->second.push_back(ep);
				}
				unlock(own_list_mutex_);

				set_event(evt_);
			}

		private :
			void i_on_thread()
			{
				try
				{
					io_.run();
				}
				catch (...)
				{
					FILE* file = fopen("C:\\AsioExcpt.txt", "ab");
					if (file)
					{
						SYSTEMTIME st = {};
						GetLocalTime(&st);
						char msg[64] = {};
						sprintf(msg, "%04d-%02d-%02d %02d:%02d:%02d Asio Exception\r\n", st.wYear, st.wMonth, st.wDay,
							st.wHour, st.wMinute, st.wSecond);
						fwrite(msg, 1, strlen(msg), file);
						fclose(file);
					}
				}
			}

			//	函数对象执行线程
			//	可能会有多个线程，从同一个优先队列
			//	中取出一个对象执行，以对象被创建
			//	的时间计算优先级，创建时间早的对象
			//	拥有较高优先级，会优先被选出执行
			void i_on_thread_get(pt_param* pt)
			{
				if (!pt) return;
				while (pt->alive)
				{
					exe_param* ep;

					lock(list_mutex_);
					if (event_list_.size())
					{
						ep = event_list_.top();
						event_list_.pop();
						unlock(list_mutex_);
					}
					else
					{
						unlock(list_mutex_);
						event_wait(pt->evt);
						continue;
					}

					auto tmp_log_fo = [](const char* info) {
						FILE* file = fopen("C:\\ThreadPoolExcpt.txt", "ab");
						if (file)
						{
							SYSTEMTIME st = {};
							GetLocalTime(&st);
							char msg[1024] = {};
							sprintf(msg, "%04d-%02d-%02d %02d:%02d:%02d %s\r\n", st.wYear, st.wMonth, st.wDay,
								st.wHour, st.wMinute, st.wSecond, info);
							fwrite(msg, 1, strlen(msg), file);
							fclose(file);
						}
					};

					try
					{
						ep->fo();
					}
					catch (std::exception& e)
					{
						tmp_log_fo(e.what());
					}
					catch (...)
					{
						tmp_log_fo("Thread Pool Exception");
					}

					ep->strand->set_using(false);
					delete ep;
				}
			}

			//	单独一个线程用于建立优先队列
			//	属于同一个 strand 的函数对象总是
			//	互斥的，不会同时出现在优先队列中
			void i_on_thread_build()
			{
				auto iter = own_event_list_.begin();

				while (build_thread_alive_)
				{
					lock(own_list_mutex_);
					for (iter = own_event_list_.begin(); iter != own_event_list_.end(); )
					{
						strand_t* strd = iter->first;
						if (!strd->is_using())
						{
							exe_param* ep;
							strd->set_using(true);

							if (iter->second.size())
							{
								ep = iter->second.front();
								iter->second.pop_front();
								if (iter->second.empty()) {
									own_event_list_.erase(iter++);
								} else {
									++iter;
								}
							}
							else
							{
								strd->set_using(false);
								own_event_list_.erase(iter++);
								unlock(own_list_mutex_);
								continue;
							}

							lock(list_mutex_);
							event_list_.push(ep);
							unlock(list_mutex_);
							for (int i = 0; i < INNER_THREAD_COUNT; i++)
							{
								set_event(post_thread_[i].evt);
							}
						}
						else
						{
							++iter;
						}
					}

					unlock(own_list_mutex_);

					if (own_event_list_.empty())
					{
						event_wait(evt_);
						continue;
					}
				}
			}

		public :
			static thread_pool_t* instance()
			{
				static thread_pool_t self;
				return &self;
			}

		private :
			int											tc_;
			asio::io_service							io_;
			std::unique_ptr<asio::io_service::work>		worker_;
			std::unique_ptr<std::thread>				threads_[MAX_THREAD_COUNT];

			int											count_;
			HEVENT										evt_;
			HMUTEX										list_mutex_;
			HMUTEX										own_list_mutex_;
			pt_param									post_thread_[INNER_THREAD_COUNT];
			std::unique_ptr<std::thread>				build_thread_;
			bool										build_thread_alive_;
			std::map<strand_t*, std::list<exe_param*>>	own_event_list_;
			std::priority_queue<exe_param*, std::vector<exe_param*>, exe_cmp> event_list_;
		};
	}

	static detail::thread_pool_t* default_thread_pool() { return detail::thread_pool_t::instance(); }
}

#endif

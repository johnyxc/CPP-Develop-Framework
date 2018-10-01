#ifndef __TIMER_IMPL_HPP_2017_10_17__
#define __TIMER_IMPL_HPP_2017_10_17__
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <list>
#include <map>
#include <windows.h>

#pragma warning(disable:4099)
#pragma warning(disable:4244)

/*
*	根据机器配置，计时精度可能会有少许误差
*/

namespace bas
{
	using func = std::function<void (int err)>;

	namespace detail
	{
		enum TIMER_IMPL_ERR
		{
			TIE_OK = 0,		//	成功
			TIE_CANCELLED	//	已取消
		};

		enum TIMER_PARAM
		{
			TP_ADJUST_DELTA = 0		//	误差调整系数
		};

		struct time_generator_t
		{
		public:
			time_generator_t();
			~time_generator_t();

		public:
			void init()
			{
				start_time_ = GetTickCount64();
			}

			void uninit()
			{
			}

			static long long get_current_time()
			{
				return GetTickCount64();
			}

		private:
			long long start_time_;
		};
		//////////////////////////////////////////////////////////////////////////

		struct absolute_time
		{
		};
		//////////////////////////////////////////////////////////////////////////

		struct relative_time
		{
		public:
			relative_time(long long ms) : ms_(ms) {}
			long long get_deadline_time() { return time_generator_t::get_current_time() + ms_; }

		private:
			long long ms_;
		};
		//////////////////////////////////////////////////////////////////////////

		struct timer_task
		{
			long long sid;
			long long timeout;
			func cb;
		};

		using post_callback		= std::function<void (timer_task)>;
		using cancel_callback	= std::function<void (long long)>;

		struct relative_timer_t
		{
			friend class relative_timer_manager_t;

		public:
			relative_timer_t(long long sid) : sid_(sid) {}
			~relative_timer_t() {}

		public:
			void wait(func cb, int ms)
			{
				timer_task task = {};
				task.sid		= sid_;
				task.cb			= cb;
				task.timeout	= time_generator_t::get_current_time() + ms;
				if (post_cb_) post_cb_(task);
			}

			void wait(func cb, relative_time rt)
			{
				timer_task task = {};
				task.sid = sid_;
				task.cb = cb;
				task.timeout = rt.get_deadline_time();
				if (post_cb_) post_cb_(task);
			}

			void wait(func cb, absolute_time at) {}	//	暂不支持

			void cancel()
			{
				if (cancel_cb_) cancel_cb_(sid_);
			}

		private:
			void i_set_post_callback(post_callback cb)
			{
				post_cb_ = cb;
			}

			void i_set_cancel_callback(cancel_callback cb)
			{
				cancel_cb_ = cb;
			}

			long long i_get_sid()
			{
				return sid_;
			}

		private:
			post_callback	post_cb_;
			cancel_callback cancel_cb_;
			long long		sid_;
		};
		//////////////////////////////////////////////////////////////////////////

		struct relative_timer_manager_t
		{
		public:
			static relative_timer_manager_t* instance()
			{
				static relative_timer_manager_t self;
				return &self;
			}

		public:
			void init()
			{
				run_		  = true;
				event_		  = CreateEvent(NULL, FALSE, FALSE, NULL);
				run_event_	  = CreateEvent(NULL, FALSE, FALSE, NULL);
				cancel_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);

				trd_	 = std::make_shared<std::thread>(
					std::bind(&relative_timer_manager_t::i_on_task_thread, this));
				cel_trd_ = std::make_shared<std::thread>(
					std::bind(&relative_timer_manager_t::i_on_cancel_thread, this));
			}

			void uninit()
			{
				run_ = false;

				{
					std::lock_guard<std::recursive_mutex> lock(mutex_);
					exit_ = true;
					sid_map_.clear();
					time_wait_map_.clear();
					cancel_list_.clear();
				}

				SetEvent(event_);
				SetEvent(run_event_);
				SetEvent(cancel_event_);

				if (trd_->joinable()) trd_->join();
				if (cel_trd_->joinable()) cel_trd_->join();
			}

			std::shared_ptr<relative_timer_t> get_timer()
			{
				std::shared_ptr<relative_timer_t> timer = std::make_shared<relative_timer_t>(InterlockedIncrement64(&sid_));
				timer->i_set_post_callback(std::bind(&relative_timer_manager_t::i_post_task, this, std::placeholders::_1));
				timer->i_set_cancel_callback(std::bind(&relative_timer_manager_t::i_cancel, this, std::placeholders::_1));
				return timer;
			}

		private:
			relative_timer_manager_t() : sid_(), exit_(false) {}
			~relative_timer_manager_t() {}

			void i_post_task(timer_task task)
			{
				{
					std::lock_guard<std::recursive_mutex> lock(mutex_);
					if (exit_) return;
					if (sid_map_.find(task.sid) != sid_map_.end()) i_cancel(task.sid);

					long long tmp_timeout = -1;
					if (!time_wait_map_.empty())
					{
						tmp_timeout = time_wait_map_.begin()->first;
					}
					sid_map_[task.sid] = task.timeout;
					time_wait_map_[task.timeout].push_back(task);

					if (task.timeout < tmp_timeout)
					{
						SetEvent(event_);
					}
				}

				SetEvent(run_event_);
			}

			void i_cancel(long long sid)
			{
				do 
				{
					std::lock_guard<std::recursive_mutex> lock(mutex_);
					if (exit_) return;
					if (sid_map_.find(sid) == sid_map_.end()) break;

					long long tmp_time = sid_map_[sid];
					sid_map_.erase(sid);

					if (time_wait_map_.find(tmp_time) == time_wait_map_.end()) break;
					auto& wait_list = time_wait_map_[tmp_time];
					for (auto iter = wait_list.begin(); iter != wait_list.end(); ++iter)
					{
						if (iter->sid == sid)
						{
							cancel_list_.push_back(*iter);
							wait_list.erase(iter);
							SetEvent(cancel_event_);
							break;
						}
					}

					if (time_wait_map_[tmp_time].empty()) time_wait_map_.erase(tmp_time);
				} while (0);

				SetEvent(run_event_);
			}

			void i_on_task_thread()
			{
				while (run_)
				{
					static long long tmp_time = -1;
					static std::list<timer_task> tmp_list;

					if (tmp_time == -1)
					{
						std::lock_guard<std::recursive_mutex> lock(mutex_);
						if (!time_wait_map_.empty())
						{
							tmp_time = time_wait_map_.begin()->first;
						}
					}

					if (tmp_time != -1)
					{
						int diff = tmp_time - time_generator_t::get_current_time();
						diff -= TP_ADJUST_DELTA;
						if (diff < 0) diff = 0;
						DWORD ret = WaitForSingleObject(event_, diff);
						if (ret == WAIT_TIMEOUT)
						{
							{
								std::lock_guard<std::recursive_mutex> lock(mutex_);
								tmp_list = time_wait_map_[tmp_time];
								time_wait_map_.erase(tmp_time);
							}

							for (auto& item : tmp_list)
							{
								if (item.cb) item.cb(TIE_OK);
							}
						}

						tmp_time = -1;
						tmp_list.clear();
					}

					if (time_wait_map_.empty())
					{
						WaitForSingleObject(run_event_, INFINITE);
					}
				}
			}

			void i_on_cancel_thread()
			{
				while (run_)
				{
					std::list<timer_task> cancel_list;

					{
						std::lock_guard<std::recursive_mutex> lock(mutex_);
						if (!cancel_list_.empty())
						{
							cancel_list = cancel_list_;
							cancel_list_.clear();
						}
					}

					for (auto& item : cancel_list)
					{
						if (item.cb) item.cb(TIE_CANCELLED);
					}

					if (cancel_list_.empty())
					{
						WaitForSingleObject(cancel_event_, INFINITE);
					}
				}
			}

		private:
			std::shared_ptr<std::thread> trd_;
			std::shared_ptr<std::thread> cel_trd_;

			bool		exit_;
			bool		run_;
			HANDLE		event_;
			HANDLE		run_event_;
			HANDLE		cancel_event_;
			long long	sid_;

			std::recursive_mutex mutex_;
			std::map<long long, long long> sid_map_;						//	KEY:sid VALUE:cur_time
			std::map<long long, std::list<timer_task>> time_wait_map_;		//	KEY:cur_time VALUE:wait_list
			std::list<timer_task> cancel_list_;
		};
	}
}

#endif

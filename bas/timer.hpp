#ifndef __TIMER_HPP_2016_09_23__
#define __TIMER_HPP_2016_09_23__
//#include <asio/asio.hpp>
//#include <bas/thread_pool.hpp>
#include <bas/timer_impl.hpp>
#include <functional>
#include <memory>

//	定时器使用限制：
//	建议一个定时器对象只与一种定时逻辑绑定
//	务必确保在对象析构前已经调用过 cancel

namespace bas
{
	namespace detail
	{
		/*
		struct timer_t :
			std::enable_shared_from_this<timer_t>
		{
		public :
			timer_t() : timer_(default_thread_pool()->get_service()) {}
			~timer_t() {}

		public :
			void wait(std::function<void ()> cb, int ms)
			{
				timer_.expires_from_now(std::chrono::milliseconds(ms));
				auto retain_this = shared_from_this();
				timer_.async_wait([retain_this, cb] (const std::error_code &err) {
					if (!err) cb();
				});
			}

			void cancel()
			{
				timer_.cancel();
			}

		private :
			asio::steady_timer timer_;
		};
		*/

		struct timer_t :
			std::enable_shared_from_this<timer_t>
		{
		public:
			timer_t() { timer_ = relative_timer_manager_t::instance()->get_timer(); }
			~timer_t() {}

		public:
			void wait(std::function<void()> cb, int ms)
			{
				timer_->wait(std::bind(&timer_t::i_on_timer, shared_from_this(), std::placeholders::_1, cb), ms);
			}

			void cancel()
			{
				timer_->cancel();
			}

		private:
			void i_on_timer(int err, std::function<void()> cb)
			{
				if (err == TIE_OK) cb();
			}

		private:
			std::shared_ptr<relative_timer_t> timer_;
		};
	}
}

#endif

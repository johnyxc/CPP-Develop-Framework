#ifndef __STRAND_HPP_2021_06_04__
#define __STRAND_HPP_2021_06_04__
#include <functional>
#include <bas/osfunc.hpp>

namespace bas
{
	namespace detail
	{
		struct strand_t
		{
		public :
			strand_t() : mutex_(), in_use_(0) { mutex_ = get_mutex(); }
			~strand_t() { release_mutex(mutex_); }

		public :
			void enter_section()
			{
				lock(mutex_);
			}

			bool try_enter_section()
			{
				return try_lock(mutex_);
			}

			int get_section_own_count()
			{
				return get_own_count(mutex_);
			}

			void set_using(bool b)
			{
				long in_use = b ? 1 : 0;
				atom_exchange(&in_use_, in_use);
			}

			bool is_using()
			{
				auto in_use = false;
				if (InterlockedCompareExchange(&in_use_, 1, 0)) in_use = true;
				return in_use;
			}

			void leave_section()
			{
				unlock(mutex_);
			}

		private :
			HMUTEX	mutex_;
			long	in_use_;
		};
	}
}

#endif

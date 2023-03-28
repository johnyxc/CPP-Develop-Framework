#ifndef __STRAND_HPP_2016_09_22__
#define __STRAND_HPP_2016_09_22__
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
				atom_exchange(&in_use_, b ? 1 : 0);
			}

			bool is_using()
			{
				return (bool)InterlockedCompareExchange(&in_use_, 0, 0);
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

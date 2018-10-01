#ifndef __LOG_H_2017_01_13__
#define __LOG_H_2017_01_13__
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <windows.h>
#include <atlstr.h>

namespace jf_log
{
	enum LOG_TYPE
	{
		LT_INFO,
		LT_ERROR,
		LT_WARNING,
		LT_DEBUG
	};

	struct LOG_INIT_INFO
	{
		std::string					name;
		std::string					path;
		std::vector<std::string>	type;
		int							expired;
		bool						unicode;
	};

	const static char log_type_string[][10] = {
		"INFO",
		"ERROR",
		"WARNING",
		"DEBUG"
	};

	const static wchar_t log_type_string_w[][10] = {
		L"INFO",
		L"ERROR",
		L"WARNING",
		L"DEBUG"
	};

	struct log_end_symbol
	{};
}

#define LOGGER_COUNT	4
//////////////////////////////////////////////////////////////////////////

struct jf_log_parent
{
public:
	jf_log_parent(jf_log::LOG_TYPE lt);
	~jf_log_parent();

public:
	void reset_mutex(std::recursive_mutex* mtx);
	void reset_fd(FILE* fd);
	void set_forbidden(bool b);

protected:
	FILE*					fd_;
	bool					forbidden_;
	jf_log::LOG_TYPE		type_;
	std::recursive_mutex*	mutex_;
};

//	默认多字节版本
struct jf_log_t : jf_log_parent
{
public:
	jf_log_t(jf_log::LOG_TYPE lt);
	~jf_log_t();

public:
	void write_log(const char* format, ...);

	template <typename T>
	jf_log_t& operator << (const T* v) { char x[0]; return *this; }

	template <typename T>
	jf_log_t& operator << (const T& v) { char x[0]; return *this; }

	template <>
	jf_log_t& operator << (const char* v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_) log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t& operator << (const std::string& v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_) log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t& operator << (const std::map<std::string, std::string>& v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_)
		{
			for (auto& item : v)
			{
				log_content_ += item.first;
				log_content_ += " = ";
				log_content_ += item.second;
				log_content_ += " ";
			}
		}

		return *this;
	}

	template <>
	jf_log_t& operator << (const int& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_string(v);

		return *this;
	}

	template <>
	jf_log_t& operator << (const unsigned int& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_string(v);

		return *this;
	}

	template <>
	jf_log_t& operator << (const long long& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_string(v);

		return *this;
	}

	template <>
	jf_log_t& operator << (const unsigned long long& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_string(v);

		return *this;
	}

	template <>
	jf_log_t& operator << (const double& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const jf_log::log_end_symbol& les)
	{
		if (!forbidden_)
		{
			std::lock_guard<std::recursive_mutex> lock(*mutex_);
			std::string hdr = i_get_log_head(jf_log::log_type_string[(int)type_]);
			hdr += log_content_;
			hdr += "\r\n";

			if (fd_)
			{
				fwrite(hdr.c_str(), 1, hdr.length(), fd_);
				//fflush(fd_);
			}

			log_content_.clear();
		}

		return *this;
	}

private:
	std::string i_get_log_head(const std::string& ti);

private:
	std::string log_content_;
};
//////////////////////////////////////////////////////////////////////////

//	宽字符版本
struct jf_log_t_w : jf_log_parent
{
public:
	jf_log_t_w(jf_log::LOG_TYPE lt);
	~jf_log_t_w();

public:
	void write_log(const wchar_t* format, ...);

	template <typename T>
	jf_log_t_w& operator << (const T* v) { char x[0]; return *this; }

	template <typename T>
	jf_log_t_w& operator << (T& v) { char x[0]; return *this; }

	template <typename T>
	jf_log_t_w& operator << (const T& v) { char x[0]; return *this; }

	template <>
	jf_log_t_w& operator << (const wchar_t* v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_) log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const std::wstring& v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_) log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const std::map<std::wstring, std::wstring>& v)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		if (!forbidden_)
		{
			for (auto& item : v)
			{
				log_content_ += item.first;
				log_content_ += L" = ";
				log_content_ += item.second;
				log_content_ += L" ";
			}
		}

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const int& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_wstring(v);

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const unsigned int& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_wstring(v);

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const long long& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_wstring(v);

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const unsigned long long& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_wstring(v);

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const double& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (CString& v)
	{
		if (forbidden_) return *this;

		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		log_content_ += v.GetBuffer(v.GetLength());
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const jf_log::log_end_symbol& les)
	{
		if (!forbidden_)
		{
			std::lock_guard<std::recursive_mutex> lock(*mutex_);
			std::wstring hdr = i_get_log_head(jf_log::log_type_string_w[(int)type_]);
			hdr += log_content_;
			hdr += L"\r\n";

			if (fd_)
			{
				fwrite(hdr.c_str(), sizeof(wchar_t), hdr.length(), fd_);
				//fflush(fd_);
			}

			log_content_.clear();
		}

		return *this;
	}

private:
	std::wstring i_get_log_head(const std::wstring& ti);

private:
	std::wstring log_content_;
};
//////////////////////////////////////////////////////////////////////////

struct jf_log_manager_t
{
public:
	static jf_log_manager_t* instance();
	bool init(const jf_log::LOG_INIT_INFO& info);
	bool uninit();

public:
	jf_log_t* get_jf_log(jf_log::LOG_TYPE type);
	jf_log_t_w* get_jf_log_w(jf_log::LOG_TYPE type);

private:
	void i_open_log_file();
	void i_close_log_file();
	void i_check_expired();
	void i_collect_files(std::vector<std::string>& files);
	void i_check_path(const std::string& path);
	time_t i_file_time_to_time_t(const FILETIME& ft);

private:
	jf_log_manager_t();
	~jf_log_manager_t();

private:
	jf_log::LOG_INIT_INFO			info_;
	SYSTEMTIME						cur_time_;
	std::shared_ptr<jf_log_t>		logger_[5];
	std::shared_ptr<jf_log_t_w>		logger_w_[5];
	FILE*							cur_fd_;
	std::recursive_mutex			mutex_;
	bool							run_;
};
//////////////////////////////////////////////////////////////////////////

//	type 参见 LOG_TYPE

//	多字节
#define LOG(type)	\
	(*jf_log_manager_t::instance()->get_jf_log(type))

//	UNICODE
#define LOGW(type)	\
	(*jf_log_manager_t::instance()->get_jf_log_w(type))

#define LOG_END jf_log::log_end_symbol()

#endif

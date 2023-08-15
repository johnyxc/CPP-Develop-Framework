#ifndef __LOG_H_2017_01_13__
#define __LOG_H_2017_01_13__
#include <string>
#include <vector>
#include <map>
#include <list>
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

	struct log_end_and_flush_symbol
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
	bool get_forbidden();
	void flush_log();

	template <typename T>
	void write_log(const T& cont);

private:
	FILE*					fd_;
	bool					forbidden_;
	jf_log::LOG_TYPE		type_;
	std::recursive_mutex*	mutex_;
};

//	默认多字节版本
struct jf_log_t
{
public:
	jf_log_t(jf_log::LOG_TYPE lt, jf_log_parent* pa);
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
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t& operator << (const std::string& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t& operator << (const std::map<std::string, std::string>& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		for (auto& item : v)
		{
			log_content_ += item.first;
			log_content_ += " = ";
			log_content_ += item.second;
			log_content_ += " ";
		}
		return *this;
	}

	template <>
	jf_log_t& operator << (const int& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const unsigned int& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const unsigned long& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const long long& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const unsigned long long& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const double& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_string(v);
		return *this;
	}

	template <>
	jf_log_t& operator << (const jf_log::log_end_symbol& les)
	{
		if (jf_parent_->get_forbidden()) return *this;

		std::string hdr = i_get_log_head(jf_log::log_type_string[(int)type_]);
		hdr += log_content_;
		hdr += "\r\n";
		jf_parent_->write_log(hdr);
		log_content_.clear();

		return *this;
	}

	template <>
	jf_log_t& operator << (const jf_log::log_end_and_flush_symbol& les)
	{
		*this << jf_log::log_end_symbol();
		jf_parent_->flush_log();
		return *this;
	}

private:
	std::string i_get_log_head(const std::string& ti);

private:
	std::string		 log_content_;
	jf_log_parent*	 jf_parent_;
	jf_log::LOG_TYPE type_;
};
//////////////////////////////////////////////////////////////////////////

//	宽字符版本
struct jf_log_t_w
{
public:
	jf_log_t_w(jf_log::LOG_TYPE lt, jf_log_parent* pa);
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
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const std::wstring& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += v;
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const std::map<std::wstring, std::wstring>& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		for (auto& item : v)
		{
			log_content_ += item.first;
			log_content_ += L" = ";
			log_content_ += item.second;
			log_content_ += L" ";
		}
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const int& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const unsigned int& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const long long& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const unsigned long long& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const double& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += std::to_wstring(v);
		return *this;
	}

	template <>
	jf_log_t_w& operator << (CString& v)
	{
		if (jf_parent_->get_forbidden()) return *this;
		log_content_ += v.GetBuffer(v.GetLength());
		return *this;
	}

	template <>
	jf_log_t_w& operator << (const jf_log::log_end_symbol& les)
	{
		if (jf_parent_->get_forbidden()) return *this;

		std::wstring hdr = i_get_log_head(jf_log::log_type_string_w[(int)type_]);
		hdr += log_content_;
		hdr += L"\r\n";
		jf_parent_->write_log(hdr);
		log_content_.clear();

		return *this;
	}

	template <>
	jf_log_t_w& operator << (const jf_log::log_end_and_flush_symbol& les)
	{
		*this << jf_log::log_end_symbol();
		jf_parent_->flush_log();
		return *this;
	}

private:
	std::wstring i_get_log_head(const std::wstring& ti);

private:
	std::wstring	 log_content_;
	jf_log_parent*	 jf_parent_;
	jf_log::LOG_TYPE type_;
};
//////////////////////////////////////////////////////////////////////////

struct jf_log_manager_t
{
public:
	static jf_log_manager_t* instance();
	bool init(const jf_log::LOG_INIT_INFO& info);
	bool uninit();

public:
	jf_log_t get_jf_log(jf_log::LOG_TYPE type);
	jf_log_t_w get_jf_log_w(jf_log::LOG_TYPE type);

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
	std::shared_ptr<jf_log_parent>	logger_[5];
	FILE*							cur_fd_;
	std::recursive_mutex			mutex_;
	bool							run_;
};
//////////////////////////////////////////////////////////////////////////

/*
	@param _folder			日志保存的目录
	@param _log_name		日志文件名
*/
bool Init_Log(const std::string& _folder, const std::string& _log_name);

/*
	@param type			LT_DEBUG, LT_INFO, LT_WARNING, LT_ERROR
*/
//	type 参见 LOG_TYPE

//	多字节
#define LOG(type)	\
	jf_log_manager_t::instance()->get_jf_log(type)

//	UNICODE
#define LOGW(type)	\
	jf_log_manager_t::instance()->get_jf_log_w(type)

//	本条日志结束
#define LOG_END jf_log::log_end_symbol()

//	本条日志结束并立即落盘
#define LOG_FLUSH jf_log::log_end_and_flush_symbol()

#endif

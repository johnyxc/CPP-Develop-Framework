#include "stdafx.h"
#include "Log.h"
#include <io.h>
#include <filesystem>

jf_log_parent::jf_log_parent(jf_log::LOG_TYPE lt) : fd_(), forbidden_(true), type_(lt), mutex_() {}
jf_log_parent::~jf_log_parent() {}

void jf_log_parent::reset_mutex(std::recursive_mutex* mtx)
{
	mutex_ = mtx;
}

void jf_log_parent::reset_fd(FILE* fd)
{
	fd_ = fd;
}

void jf_log_parent::set_forbidden(bool b)
{
	forbidden_ = b;
}

bool jf_log_parent::get_forbidden()
{
	return forbidden_;
}

template <typename T1, typename T2>
void jf_log_parent::write_log(const T1& cont)
{
	if (fd_)
	{
		std::lock_guard<std::recursive_mutex> lock(*mutex_);
		fwrite(cont.c_str(), sizeof(T2), cont.length(), fd_);
	}
}
//////////////////////////////////////////////////////////////////////////

jf_log_t::jf_log_t(jf_log::LOG_TYPE lt, jf_log_parent* pa) : type_(lt), jf_parent_(pa) {}
jf_log_t::~jf_log_t() {}

void jf_log_t::write_log(const char* format, ...)
{
	if (jf_parent_->get_forbidden()) return;

	va_list args;
	va_start(args, format);

	std::string hdr = i_get_log_head(jf_log::log_type_string[(int)type_]);
	char dest[1024] = {};
	_vsnprintf(dest, sizeof(dest), format, args);
	hdr += dest;
	hdr += "\r\n";

	jf_parent_->write_log(hdr);

	va_end(args);
}

std::string jf_log_t::i_get_log_head(const std::string& ti)
{
	SYSTEMTIME st = {};
	GetLocalTime(&st);

	std::string res;
	char dest[512] = {};
	sprintf(dest, "[%s] %04d-%02d-%02d %02d:%02d:%02d ", ti.c_str(), st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	res = dest;

	return std::move(res);
}
//////////////////////////////////////////////////////////////////////////

jf_log_t_w::jf_log_t_w(jf_log::LOG_TYPE lt, jf_log_parent* pa) : type_(lt), jf_parent_(pa) {}
jf_log_t_w::~jf_log_t_w() {}

void jf_log_t_w::write_log(const wchar_t* format, ...)
{
	if (jf_parent_->get_forbidden()) return;

	va_list args;
	va_start(args, format);

	std::wstring hdr = i_get_log_head(jf_log::log_type_string_w[(int)type_]);
	wchar_t dest[1024] = {};
	_vsnwprintf(dest, sizeof(dest), format, args);
	hdr += dest;
	hdr += L"\r\n";

	jf_parent_->write_log<std::wstring, wchar_t>(hdr);

	va_end(args);
}

std::wstring jf_log_t_w::i_get_log_head(const std::wstring& ti)
{
	SYSTEMTIME st = {};
	GetLocalTime(&st);

	std::wstring res;
	wchar_t dest[512] = {};
	wsprintfW(dest, L"[%s] %04d-%02d-%02d %02d:%02d:%02d ", ti.c_str(), st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	res = dest;

	return std::move(res);
}
//////////////////////////////////////////////////////////////////////////

jf_log_manager_t* jf_log_manager_t::instance()
{
	static jf_log_manager_t mgr;
	return &mgr;
}

bool jf_log_manager_t::init(const jf_log::LOG_INIT_INFO& info)
{
	info_ = info;
	GetLocalTime(&cur_time_);
	i_open_log_file();

	if (info_.expired) i_check_expired();

	for (int i = 0; i < LOGGER_COUNT; i++)
	{
		logger_[i] = std::make_shared<jf_log_parent>((jf_log::LOG_TYPE)i);
		logger_[i]->reset_mutex(&mutex_);
		logger_[i]->reset_fd(cur_fd_);
		if (std::find(info_.type.begin(), info_.type.end(), jf_log::log_type_string[i])
			!= info_.type.end())
		{
			logger_[i]->set_forbidden(false);
		}
	}

	std::thread trd([this]() {
		while (run_)
		{
			if (cur_fd_) fflush(cur_fd_);

			SYSTEMTIME st;
			GetLocalTime(&st);

			if (cur_time_.wYear != st.wYear ||
				cur_time_.wMonth != st.wMonth ||
				cur_time_.wDay != st.wDay)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				i_close_log_file();
				cur_time_ = st;
				i_open_log_file();

				for (int i = 0; i < LOGGER_COUNT; i++)
				{
					logger_[i]->reset_fd(cur_fd_);
				}

				if (info_.expired) i_check_expired();
			}

			Sleep(2000);
		}
	});
	trd.detach();

	return true;
}

bool jf_log_manager_t::uninit()
{
	i_close_log_file();
	run_ = false;
	return true;
}

jf_log_t jf_log_manager_t::get_jf_log(jf_log::LOG_TYPE type)
{
	return jf_log_t(type, logger_[(int)type].get());
}

jf_log_t_w jf_log_manager_t::get_jf_log_w(jf_log::LOG_TYPE type)
{
	return jf_log_t_w(type, logger_[(int)type].get());
}

void jf_log_manager_t::i_open_log_file()
{
	char dest[128] = {};
	sprintf(dest, "%s_%04d_%02d_%02d.log", info_.name.c_str(),
		cur_time_.wYear, cur_time_.wMonth, cur_time_.wDay);

	i_check_path(info_.path);
	std::string log_path = info_.path;
	log_path += "\\";
	log_path += dest;

	cur_fd_ = fopen(log_path.c_str(), "ab");
}

void jf_log_manager_t::i_close_log_file()
{
	if (cur_fd_)
	{
		fclose(cur_fd_);
		cur_fd_ = 0;
	}
}

void jf_log_manager_t::i_check_expired()
{
	std::vector<std::string> file_list;
	std::map<time_t, std::string> file_map;
	i_collect_files(file_list);

	while (!file_list.empty())
	{
		auto fname = std::move(file_list.back());
		file_list.pop_back();

		WIN32_FILE_ATTRIBUTE_DATA attr = {};
		if (GetFileAttributesExA(fname.c_str(), GetFileExInfoStandard, &attr))
		{
			file_map[i_file_time_to_time_t(attr.ftCreationTime)] = std::move(fname);
		}
	}

	while (file_map.size() > info_.expired)
	{
		auto del_iter = file_map.begin();
		if (!del_iter->second.empty())
		{
			DeleteFileA(del_iter->second.c_str());
			file_map.erase(del_iter);
		}
	}
}

void jf_log_manager_t::i_collect_files(std::vector<std::string>& files)
{
	std::tr2::sys::path file_path(info_.path);
	std::tr2::sys::directory_iterator iter(file_path);
	std::tr2::sys::directory_iterator end_iter;
	for (; iter != end_iter; ++iter)
	{
		auto& fname = iter->path();
		if (fname.filename().find(info_.name) != fname.filename().npos)
		{
			files.emplace_back(fname);
		}
	}
}

void jf_log_manager_t::i_check_path(const std::string& path)
{
	std::vector<std::string> stack;
	stack.emplace_back(path);

	std::tr2::sys::path pt(path);
	pt = pt.parent_path();

	while (!pt.empty())
	{
		stack.emplace_back(pt);
		pt = pt.parent_path();
	}

	while (!stack.empty())
	{
		if (_access(stack.back().c_str(), 0) == -1) CreateDirectoryA(stack.back().c_str(), 0);
		stack.pop_back();
	}
}

time_t jf_log_manager_t::i_file_time_to_time_t(const FILETIME& ft)
{
	LONGLONG ll;
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;
	ll = ((ULONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
	return ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
}

jf_log_manager_t::jf_log_manager_t() : run_(true) {}
jf_log_manager_t::~jf_log_manager_t() {}


/*
@param _folder			日志保存的目录
@param _log_name		日志文件名
*/
bool Init_Log(const std::string& _folder, const std::string& _log_name)
{
	std::vector<std::string> log_type = { "DEBUG", "INFO", "WARNING", "ERROR" };
	jf_log::LOG_INIT_INFO init_info = { _log_name, _folder, log_type, 7, false };
	auto logger = jf_log_manager_t::instance();
	return logger->init(init_info);
}
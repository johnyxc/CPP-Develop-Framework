#ifndef __FILE_GENERATOR_HPP_2017_01_20__
#define __FILE_GENERATOR_HPP_2017_01_20__
#include <string>
#include <time.h>
extern std::string g_pkg_name;

struct file_generator_t
{
public:
	file_generator_t() {}
	virtual ~file_generator_t() {}

	virtual bool init() { return false; }

	void input_string(const std::string& str)
	{
		if (str.empty()) return;
		final_file_ += str;
	}

	void input_line(const std::string& str)
	{
		if (str.empty()) return;
		final_file_ += str;
		final_file_ += "\r\n";
	}

	void output(const std::string& path)
	{
		FILE* file = fopen(path.c_str(), "wb");
		if(file)
		{
			fwrite(final_file_.c_str(), 1, final_file_.length(), file);
			fclose(file);
		}

		final_file_.clear();
	}

private:
	std::string final_file_;
};
//////////////////////////////////////////////////////////////////////////

struct cpp_generator_t : file_generator_t
{
	bool init() override
	{
		input_line("#pragma once");
		input_string("\r\n");

		return true;
	}
};

struct go_generator_t : file_generator_t
{
	bool init() override
	{
		std::string input = "package ";
		input += g_pkg_name;
		input_line(input);
		input_string("\r\n");

		return true;
	}
};

#endif

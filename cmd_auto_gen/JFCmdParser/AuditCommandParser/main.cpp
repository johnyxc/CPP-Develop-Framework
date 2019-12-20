#include <string>
#include <assert.h>
#include "CmdParser.hpp"
#include "FileGenerator.hpp"

static std::string g_path;
static std::string g_pkg_name = "Protocol";
static command_parser_t g_parser;
static char file_type[][20] = { "-cpp_out", "-go_out", "-all_out" };

void generate_cpp_file(command_parser_t::params_t& param)
{
	auto fo = [](unsigned int value_start, unsigned int value_end,
		const std::string& file_name, command_parser_t::string_group_t& item)
	{
		std::string file_path = g_path;
		file_path += "\\";
		file_path += file_name;
		file_path += ".h";

		file_generator_t* gene = new cpp_generator_t;
		gene->init();

		for (auto& iter : item)
		{
			if (value_start >= value_end) assert(0 > 1);

			{	//	request
				char sz_value[30] = {};
				sprintf(sz_value, "0x%x", value_start);

				std::string var_item = "static const unsigned int ";
				var_item += iter;
				var_item += "_request";
				var_item += " = ";
				var_item += sz_value;
				var_item += ";";
				gene->input_line(var_item);
			}

			{	//	response
				char sz_value[30] = {};
				sprintf(sz_value, "0x%x", (value_start | 0x80000000));

				std::string var_item = "static const unsigned int ";
				var_item += iter;
				var_item += "_response";
				var_item += " = ";
				var_item += sz_value;
				var_item += ";";
				gene->input_line(var_item);
			}

			++value_start;
		}

		gene->output(file_path);
		delete gene;
	};

	if (param.size() == 0) return;

	for(auto& item : param)
	{
		if (!item.first.compare("notes"))
		{
			continue;
		}

		int begin = 0, end = 0;
		sscanf(item.second.bit_info.c_str(), "%d-%d", &begin, &end);

		unsigned int value_start = 1 << (begin - 1);
		unsigned int value_end = 1 << end;
		fo(value_start, value_end, item.first, item.second.cmd_list);
	}
}

void generate_go_file(command_parser_t::params_t& param)
{
	auto fo = [](unsigned int value_start, unsigned int value_end,
		const std::string& file_name, command_parser_t::string_group_t& item)
	{
		std::string file_path = g_path;
		file_path += "\\";
		file_path += file_name;
		file_path += ".go";

		file_generator_t* gene = new go_generator_t;
		gene->init();
		gene->input_line("const (");

		for (auto& iter : item)
		{
			if (value_start >= value_end) assert(0 > 1);

			{	//	request
				char sz_value[30] = {};
				sprintf(sz_value, "0x%x", value_start);

				std::string var_item = iter;
				if (var_item[0] >= 'a' && var_item[0] <= 'z') var_item[0] -= ('a' - 'A');
				var_item += "_request";
				var_item += " ";
				var_item += "uint32 = ";
				var_item += sz_value;
				gene->input_line(var_item);
			}

			{	//	response
				char sz_value[30] = {};
				sprintf(sz_value, "0x%x", (value_start | 0x80000000));

				std::string var_item = iter;
				if (var_item[0] >= 'a' && var_item[0] <= 'z') var_item[0] -= ('a' - 'A');
				var_item += "_response";
				var_item += " ";
				var_item += "uint32 = ";
				var_item += sz_value;
				gene->input_line(var_item);
			}

			++value_start;
		}

		gene->input_line(")");
		gene->output(file_path);
		delete gene;
	};

	if (param.size() == 0) return;

	for (auto& item : param)
	{
		if (!item.first.compare("notes"))
		{
			continue;
		}

		int begin = 0, end = 0;
		sscanf(item.second.bit_info.c_str(), "%d-%d", &begin, &end);

		unsigned int value_start = 1 << (begin - 1);
		unsigned int value_end = 1 << end;
		fo(value_start, value_end, item.first, item.second.cmd_list);
	}
}

int main(int args, char* argv[])
{
	auto cpp_fun = [](command_parser_t::params_t& param){
		generate_cpp_file(param);
	};

	auto go_fun = [](command_parser_t::params_t& param){
		generate_go_file(param);
	};

	printf("Args : %d\n", args);

	switch(args)
	{
	case 1:
		{
			g_path = ".";
			g_parser.parse_from_file();
			command_parser_t::params_t& param = g_parser.get_spec_character();
			cpp_fun(param);
			go_fun(param);
		}
		break;
	case 2:
		{
			g_path = ".";
			std::string src_path = ".\\";
			src_path += argv[1];
			g_parser.parse_from_file(src_path);
			command_parser_t::params_t& param = g_parser.get_spec_character();
			cpp_fun(param);
			go_fun(param);
		}
		break;
	case 3:
		{
			char* ptype = argv[1];
			g_path = argv[2];

			g_parser.parse_from_file();
			command_parser_t::params_t& param = g_parser.get_spec_character();

			if (!strcmp(ptype, file_type[0])) {
				cpp_fun(param);
			} else if (!strcmp(ptype, file_type[1])) {
				go_fun(param);
			} else if (!strcmp(ptype, file_type[2])) {
				cpp_fun(param);
				go_fun(param);
			}
		}
		break;
	case 4:
		{
			char* ptype = argv[1];
			g_path = argv[2];
			g_pkg_name = argv[3];

			g_parser.parse_from_file();
			command_parser_t::params_t& param = g_parser.get_spec_character();

			if (!strcmp(ptype, file_type[0])) {
				cpp_fun(param);
			} else if (!strcmp(ptype, file_type[1])) {
				go_fun(param);
			} else if (!strcmp(ptype, file_type[2])) {
				cpp_fun(param);
				go_fun(param);
			}
		}
		break;
	default:
		printf("Params Error\n");
		break;
	}

	return 0;
}

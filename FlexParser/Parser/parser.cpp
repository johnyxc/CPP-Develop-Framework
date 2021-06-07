#include "parser.h"
#include <time.h>
#include <iostream>
#include <windows.h>

static void str2upper(std::string& str)
{
	if (str.empty()) return;
	for (auto& item : str)
		item = ::toupper(item);
}

static std::string get_year_month_day(time_t time)
{
	struct tm ret_tm = {};
	struct tm* tm_time = localtime(((time_t*)&time));

	ret_tm.tm_year = tm_time->tm_year + 1900;
	ret_tm.tm_mon = tm_time->tm_mon + 1;
	ret_tm.tm_mday = tm_time->tm_mday;

	char fmt[15] = {};
	sprintf(fmt, "%04d_%02d_%02d", ret_tm.tm_year, ret_tm.tm_mon, ret_tm.tm_mday);

	return fmt;
}
//////////////////////////////////////////////////////////////////////////

//	C++
void cpp_output_t::output(node_t* root)
{
	if (!root) return;

	auto node = new node_t;
	node->node_type = NT_OPER;
	node->oper_type = OT_CLOSE;
	root->child.push_back(node);

	i_output(root);
	printf("Cpp Output Finish\n");
}

void cpp_output_t::i_output(node_t* node)
{
	if (node->child.empty())
	{
		if (node->node_type == NT_DATA)
		{
			data_nodes_.push_back(node);
		}
		else if (node->node_type == NT_OPER)
		{
			if (file_)
			{
				const char* endif = "\r\n#endif\r\n";
				fwrite(endif, 1, strlen(endif), file_);
				fclose(file_);
			}
		}
		return;
	}

	while (!node->child.empty())
	{
		auto child = node->child.front();
		node->child.pop_front();
		i_output(child);
	}

	if (node->node_type == NT_OPER)
	{
		switch (node->oper_type)
		{
		case OT_CREATE:
			{
				auto pnode = data_nodes_.front();
				data_nodes_.pop_front();

				if (!pnode->data.empty())
				{
					std::string fn = pnode->data + ".h";
					file_ = fopen(fn.c_str(), "wb");
					if (!file_)
					{
						std::cout << "Open " << pnode->data << " Error" << std::endl;
						exit(0);
					}

					auto file_header = i_get_header(pnode->data);
					if (!file_header.empty())
					{
						fwrite(file_header.c_str(), 1, file_header.length(), file_);
					}
				}
			}
			break;
		case OT_WRITE:
			{
				std::string fin_line;
				const std::string range = node->data;

				while (!data_nodes_.empty())
				{
					auto pnode = data_nodes_.front();
					data_nodes_.pop_front();

					if (pnode->data_type == DT_CMD_NAME)
					{
						auto pos = range.find("-");
						if (pos != range.npos)
						{
							auto left = range.substr(0, pos);
							pos += 1;
							auto right = range.substr(pos, range.length() - pos);
							auto left_int = atoi(left.c_str());
							auto right_int = atoi(right.c_str());

							int start_cmd = 1 << (left_int - 1);
							int end_cmd = 0;
							right_int -= 1;
							while (right_int >= 0)
							{
								end_cmd |= (1 << right_int);
								right_int -= 1;
							}

							auto cmd_id = start_cmd + pnode->sid;
							if (cmd_id > end_cmd)
							{
								std::cout << "Cmd " << pnode->data_type << " Out of Range" << std::endl;
								exit(0);
							}

							std::string param_type = "static const unsigned int ";

							{	//	Request
								fin_line += param_type + pnode->data + "_request = ";
								char hex[15] = {};
								sprintf(hex, "0x%x", cmd_id);
								fin_line += std::string(hex) + ";";
								fin_line += "\r\n";
							}

							{	//	Response
								fin_line += param_type + pnode->data + "_response = ";
								char hex[15] = {};
								sprintf(hex, "0x%x", (cmd_id | 0x80000000));
								fin_line += std::string(hex) + ";";
							}
						}
					}
					else if (pnode->data_type == DT_CMT)
					{	//	注释添加在 Response 后，utf8 转 gb2312

						auto trans = [](const std::string& utf8) -> std::string {
							auto len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
							auto* wstr = new wchar_t[len + 1];
							memset(wstr, 0, len + 1);

							MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
							len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
							auto* str = new char[len + 1];
							memset(str, 0, len + 1);

							WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
							delete[] wstr;
							std::string res = str;
							delete[] str;
							
							return std::move(res);
						};

						fin_line += "\t";
						fin_line += trans(pnode->data);
					}
				}

				if (file_)
				{
					fin_line += "\r\n";
					fwrite(fin_line.c_str(), 1, fin_line.length(), file_);
				}
			}
			break;
		default:
			break;
		}
	}
}

std::string cpp_output_t::i_get_header(const std::string& name)
{
	if (name.empty()) return std::string();

	std::string header = "#ifndef ";
	std::string tmp_name = "__" + name;

	str2upper(tmp_name);
	tmp_name += "_" + get_year_month_day(time(0)) + "__";

	header += tmp_name;
	header += "\r\n";
	header += "#define " + tmp_name;
	header += "\r\n\r\n";

	return std::move(header);
}
//////////////////////////////////////////////////////////////////////////

//	Golang
void go_output_t::output(node_t* root)
{
	if (!root) return;

	auto node = new node_t;
	node->node_type = NT_OPER;
	node->oper_type = OT_CLOSE;
	root->child.push_back(node);

	i_output(root);
	printf("Golang Output Finish\n");
}

void go_output_t::i_output(node_t* node)
{
	if (node->child.empty())
	{
		if (node->node_type == NT_DATA)
		{
			data_nodes_.push_back(node);
		}
		else if (node->node_type == NT_OPER)
		{
			if (file_)
			{
				const char* endif = ")";
				fwrite(endif, 1, strlen(endif), file_);
				fclose(file_);
			}
		}
		return;
	}

	while (!node->child.empty())
	{
		auto child = node->child.front();
		node->child.pop_front();
		i_output(child);
	}

	if (node->node_type == NT_OPER)
	{
		switch (node->oper_type)
		{
		case OT_CREATE:
			{
				auto pnode = data_nodes_.front();
				data_nodes_.pop_front();

				if (!pnode->data.empty())
				{
					std::string fn = pnode->data + ".go";
					file_ = fopen(fn.c_str(), "wb");
					if (!file_)
					{
						std::cout << "Open " << pnode->data << " Error" << std::endl;
						exit(0);
					}

					auto file_header = i_get_header(pnode->data);
					if (!file_header.empty())
					{
						fwrite(file_header.c_str(), 1, file_header.length(), file_);
					}
				}
			}
			break;
		case OT_WRITE:
			{
				std::string fin_line;
				const std::string range = node->data;

				while (!data_nodes_.empty())
				{
					auto pnode = data_nodes_.front();
					data_nodes_.pop_front();

					if (pnode->data_type == DT_CMD_NAME)
					{
						auto pos = range.find("-");
						if (pos != range.npos)
						{
							auto left = range.substr(0, pos);
							pos += 1;
							auto right = range.substr(pos, range.length() - pos);
							auto left_int = atoi(left.c_str());
							auto right_int = atoi(right.c_str());

							int start_cmd = 1 << (left_int - 1);
							int end_cmd = 0;
							right_int -= 1;
							while (right_int >= 0)
							{
								end_cmd |= (1 << right_int);
								right_int -= 1;
							}

							auto cmd_id = start_cmd + pnode->sid;
							if (cmd_id > end_cmd)
							{
								std::cout << "Cmd " << pnode->data_type << " Out of Range" << std::endl;
								exit(0);
							}

							{	//	Request
								pnode->data[0] -= 32;	//	go 定义变量首字母大写

								fin_line += pnode->data + "_request uint32 = ";
								char hex[15] = {};
								sprintf(hex, "0x%x", cmd_id);
								fin_line += std::string(hex) + ";";
								fin_line += "\r\n";
							}

							{	//	Response
								fin_line += pnode->data + "_response uint32 = ";
								char hex[15] = {};
								sprintf(hex, "0x%x", (cmd_id | 0x80000000));
								fin_line += std::string(hex) + ";";
							}	
						}
					}
					else if (pnode->data_type == DT_CMT)
					{	//	注释添加在 Response 后
						fin_line += "\t";
						fin_line += pnode->data;
					}
				}

				if (file_)
				{
					fin_line += "\r\n";
					fwrite(fin_line.c_str(), 1, fin_line.length(), file_);
				}
			}
			break;
		default:
			break;
		}
	}
}

std::string go_output_t::i_get_header(const std::string& name)
{
	if (name.empty()) return std::string();

	std::string header = "package Protocol\r\n\r\n";
	header += "const (\r\n";

	return std::move(header);
}
//////////////////////////////////////////////////////////////////////////

jf_cmd_parser_t* jf_cmd_parser_t::instance()
{
	static jf_cmd_parser_t self;
	return &self;
}

jf_cmd_parser_t::jf_cmd_parser_t() : output_(), cur_node_(), parent_node_()
{
	root_ = new node_t;
	root_->node_type = NT_ROOT;

	cur_node_ = parent_node_ = root_;
}
jf_cmd_parser_t::~jf_cmd_parser_t() {}

void jf_cmd_parser_t::set_target_type(const char* tar)
{
	if (!tar) return;

	if (g_tar_cpp == tar)
	{
		output_ = new cpp_output_t;
	}
	else if (g_tar_go == tar)
	{
		output_ = new go_output_t;
	}
}

void jf_cmd_parser_t::set_global_comment(const char* cmt)
{	//	目前直接丢弃全局注释，可考虑写入单独的 Comment.h 文件中
}

void jf_cmd_parser_t::set_single_line_comment(const char* cmt, int line)
{
	if (cur_node_ && parent_node_)
	{
		if (cur_node_->line == line)
		{	//	cur_node_ 所在行的注释
			auto node = new node_t;
			node->node_type = NT_DATA;
			node->data_type = DT_CMT;
			node->data = cmt;
			node->line = line;

			cur_node_ = node;
			parent_node_->child.push_back(node);
			return;
		}
	}

	//	丢弃不属于任何命令的单行注释
}

void jf_cmd_parser_t::set_file_name(const char* cmt)
{
	if (!cmt) return;

	if (!root_->child.empty())
	{	//	在打开新文件前关闭上一个文件
		auto close_node = new node_t;
		close_node->node_type = NT_OPER;
		close_node->oper_type = OT_CLOSE;
		root_->child.push_back(close_node);
	}

	cur_node_ = parent_node_ = root_;

	auto node = new node_t;
	node->node_type = NT_OPER;
	node->oper_type = OT_CREATE;

	auto child = new node_t;
	child->node_type = NT_DATA;
	child->data = std::string(cmt + 1, strlen(cmt) - 2);
	child->data_type = DT_FILE_NAME;

	node->child.push_back(child);
	if (parent_node_) parent_node_->child.push_back(node);

	parent_node_ = node;
	cur_node_ = child;
}

void jf_cmd_parser_t::set_bits(const char* cmt)
{
	if (!cmt) return;
	bits_ = cmt;
}

void jf_cmd_parser_t::set_cmd_name(const char* cmt, int line, int sid)
{
	if (!cmt) return;

	cur_node_ = parent_node_ = root_;

	auto node = new node_t;
	node->node_type = NT_OPER;
	node->oper_type = OT_WRITE;
	node->data = bits_.substr(1, bits_.length() - 2);
	node->data_type = DT_CMD_RANGE;

	if (parent_node_) parent_node_->child.push_back(node);
	cur_node_ = node;

	auto child = new node_t;
	child->node_type = NT_DATA;
	child->data = std::string(cmt, strlen(cmt) - 1);
	child->data_type = DT_CMD_NAME;
	child->line = line;
	child->sid = sid;

	if (cur_node_->data_type == DT_CMD_RANGE)
	{
		cur_node_->child.push_back(child);
		parent_node_ = cur_node_;
	}

	cur_node_ = child;
}

void jf_cmd_parser_t::output()
{
	if (output_) output_->output(root_);
}

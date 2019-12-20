#ifndef __CMD_PARSER_HPP_2017_01_20__
#define __CMD_PARSER_HPP_2017_01_20__
#include <map>
#include <vector>
#include <string>
#include <memory>
#pragma warning(disable : 4996)

struct ParseInfo
{
	std::string bit_info;
	std::vector<std::string> cmd_list;
};

struct command_parser_t
{
	using string_group_t	= std::vector<std::string>;
	using params_t			= std::map<std::string, ParseInfo>;
	using pair_t			= std::pair<const std::string, std::vector<std::string>>;

public:
	command_parser_t() {}
	~command_parser_t() {}

public:
	bool parse_from_file(const std::string& path = ".\\cmd_def.zdef")
	{
		FILE* file = fopen(path.c_str(), "rb");
		if (!file) return false;

		fseek(file, 0, SEEK_END);
		int fsize = ftell(file);
		rewind(file);

		std::shared_ptr<char> content(new char[fsize], [](char* p){ delete[] p; });
		fread(content.get(), 1, fsize, file);
		fclose(file);

		i_parse_stream(std::string(content.get()));

		return true;
	}

	params_t& get_spec_character()
	{
		return char_group_;
		/*
		for(auto& chara : param)
		{
			auto iter = char_group_.find(chara.first);
			if(iter != char_group_.end())
			{
				chara.second = std::move(iter->second);
			}
		}
		*/
	}

private:
	void i_parse_stream(std::string& stm)
	{
		i_find_notes(stm);
		i_parse_all_sectors(stm);
		/*
		i_find_common_command(stm);
		i_find_general_cashier_command(stm);
		i_find_general_client_command(stm);
		i_find_general_db_command(stm);
		i_find_cashier_db_command(stm);
		i_find_cashier_client_command(stm);
		i_find_client_db_command(stm);
		*/
	}

	void i_find_notes(std::string& stm)
	{
		int bpos = stm.find_first_of('/');
		if(bpos != stm.npos)
		{
			if(stm[bpos + 1] == '*')
			{	//	×¢ÊÍ¿ªÊ¼
				int epos = stm.find_first_of('/', bpos + 1);
				if(epos != stm.npos)
				{
					if(stm[epos - 1] == '*')
					{	//	×¢ÊÍ½áÊø
						std::vector<std::string> vec;
						vec.push_back(std::move(stm.substr(bpos, epos - bpos + 1)));
						char_group_["notes"] = ParseInfo { "0", vec };

						epos += 1;
						stm = stm.substr(epos, stm.length() - epos);
					}
				}
			}
		}
	}

	void i_find_common_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "common_command");
	}

	void i_find_general_cashier_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "general_cashier_command");
	}

	void i_find_general_client_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "general_client_command");
	}

	void i_find_general_db_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "general_db_command");
	}

	void i_find_cashier_db_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "cashier_db_command");
	}

	void i_find_cashier_client_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "cashier_client_command");
	}

	void i_find_client_db_command(std::string& stm)
	{
		i_find_spec_name_command(stm, "client_db_command");
	}

	bool i_find_spec_name_command(std::string& stm, const std::string& name)
	{
		if (stm.empty()) return false;

		int bpos = 0;
		do
		{
			bpos = stm.find_first_of('[', bpos);
			if (bpos != stm.npos)
			{
				bpos += 1;
				int epos = stm.find_first_of(']', bpos);
				if (epos != stm.npos)
				{
					std::string sym = stm.substr(bpos, epos - bpos);
					if (sym == name)
					{
						bpos += 1;
						bpos = stm.find_first_of('[', bpos);
						if (bpos != stm.npos)
						{
							bpos += 1;
							epos = stm.find_first_of(']', bpos);
							if (epos != stm.npos)
							{
								std::string cmd_name = stm.substr(bpos, epos - bpos);

								bpos += 1;
								bpos = stm.find_first_of('[', bpos);
								if (bpos != stm.npos)
								{
									bpos += 1;
									epos = stm.find_first_of(']', bpos);
									if (epos != stm.npos)
									{
										std::string bit_info = stm.substr(bpos, epos - bpos);
										std::string sub_cont;

										int next_pos = stm.find_first_of('[', epos);
										if (next_pos == stm.npos) {
											sub_cont = stm.substr(epos, stm.length() - epos);
										} else {
											sub_cont = stm.substr(epos, next_pos - epos);
										}

										ParseInfo pi;
										pi.bit_info = bit_info;
										i_content_strtok(sub_cont, pi.cmd_list);
										char_group_[cmd_name] = pi;
									}
								}

								return true;
							}
						}
					}
				}
			}
		} while (bpos != stm.npos);

		return false;
	}

	void i_parse_all_sectors(std::string& cont)
	{
		int sector = 1;

		bool b = i_find_spec_name_command(cont, std::to_string(sector++));
		while (b)
		{
			b = i_find_spec_name_command(cont, std::to_string(sector++));
		}
	}

	void i_content_strtok(std::string& cont, std::vector<std::string>& vec)
	{
		char* token = ";\r\n";
		char* word = strtok(const_cast<char*>(cont.c_str()), token);

		while (word != nullptr)
		{
			if((word[0] >= 'a' && word[0] <= 'z') ||
				(word[0] >= 'A' && word[0] <= 'Z')) vec.push_back(word);
			word = strtok(nullptr, token);
		}
	}

private:
	params_t char_group_;
};

#endif

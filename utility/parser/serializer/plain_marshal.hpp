#ifndef __PLAIN_MARSHAL_HPP_2020_03_27__
#define __PLAIN_MARSHAL_HPP_2020_03_27__
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

//	io 接口定义
struct io_opt_t
{
	virtual std::string read() = 0;
	virtual int write(const std::string& data) = 0;
};

//	默认文件 io 实现
struct default_file_io_t : io_opt_t
{
public:
	default_file_io_t(const std::string& path)
	{
		file_path_ = path;
	}

	std::string read() override
	{
		if (file_path_.empty()) return std::string();

		FILE* fd = fopen(file_path_.c_str(), "rb");
		if (!fd) return std::string();

		fseek(fd, 0, SEEK_END);
		auto fsize = ftell(fd);
		rewind(fd);

		std::shared_ptr<char> content(new char[fsize], [](char* p) { delete[] p; });
		fread(content.get(), 1, fsize, fd);
		fclose(fd);

		return std::string(content.get(), fsize);
	}

	int write(const std::string& data) override
	{
		if (data.empty() || file_path_.empty()) return 0;
		
		FILE* fd = fopen(file_path_.c_str(), "wb");
		if (!fd) return 0;
		
		auto bt = fwrite(data.c_str(), 1, data.length(), fd);
		fclose(fd);

		return (int)bt;
	}

private:
	std::string file_path_;
};
//////////////////////////////////////////////////////////////////////////

//	只可序列化/反序列化扁平结构体
template <typename T>
struct plain_marshal_t
{
	plain_marshal_t()
	{
		memset(this, 0, sizeof(T));
	}

	bool unmarshal(const std::string& data) const
	{
		static_assert(std::is_standard_layout<T>::value, "non-stdlayout struct is NOT acceptable");
		if (data.empty()) return false;

		auto head = data.c_str();
		auto tmp_len = 0;
		auto body_len = *(int*)head;
		auto real_len = sizeof(T);

		head += sizeof(int);

		if (body_len != real_len)
		{
			(body_len < (int)real_len) ? tmp_len = body_len : tmp_len = (int)real_len;
		}
		else
		{
			tmp_len = body_len;
		}

		memcpy((char*)this, head, tmp_len);
		return true;
	}

	std::string marshal() const
	{
		static_assert(std::is_standard_layout<T>::value, "non-stdlayout struct is NOT acceptable");
		int len = sizeof(T);

		std::string data;
		data += std::string((const char*)&len, sizeof(len));
		data += std::string((const char*)this, sizeof(T));

		return std::move(data);
	}
};
//////////////////////////////////////////////////////////////////////////

//	固定返回 std::vector
template <typename T>
static std::vector<T> unmarshal(io_opt_t* io)
{
	static_assert(std::is_standard_layout<T>::value, "non-stdlayout struct is NOT acceptable");

	std::string src = io->read();
	if (src.empty()) return std::vector<T>();

	auto head = src.c_str();
	decltype(head) tail = head + src.length();
	std::vector<T> obj_list;

	while (head < tail)
	{
		auto remain = tail - head;
		if (remain < sizeof(int)) return std::vector<T>();

		T ci;
		auto body_len = *(int*)head;
		if (body_len > remain) return std::vector<T>();

		auto block_len = body_len + sizeof(int);
		if (!ci.unmarshal(std::string(head, block_len))) return std::vector<T>();
		head += block_len;
		obj_list.push_back(ci);
	}

	return obj_list;
}

//	接受 std::vector, std::list, std::set 等可迭代容器
template <typename CT>
static bool marshal(const CT& obj, io_opt_t* io)
{
	typedef typename CT::value_type T;
	static_assert(std::is_standard_layout<T>::value, "non-stdlayout struct is NOT acceptable");

	if (!obj.size() || !io) return false;

	std::string stream;
	for (const auto& item : obj)
	{
		auto data = item.marshal();
		if (data.empty()) return false;
		stream += data;
	}

	auto bt = io->write(stream);
	if (bt != stream.length()) return false;

	return true;
}
//////////////////////////////////////////////////////////////////////////

// Example
static void example()
{
//#pragma pack(1)
	//	序列化结构体必须为标准布局，不允许包含任何 STL 容器
	//	确保序列化/反序列化时的对齐设置一致
	struct Person : plain_marshal_t<Person>
	{
		char name[10];
		int age;
		int grade;
	};

	struct PersonEx : plain_marshal_t<PersonEx>
	{
		char name[10];
		int age;
	};
//#pragma pack()

	Person p1, p2;
	strncpy(p1.name, "John", sizeof(p1.name));
	p1.age = 18;
	p1.grade = 5;

	strncpy(p2.name, "Mars", sizeof(p2.name));
	p2.age = 17;
	p2.grade = 6;

#if 0
	default_file_io_t io("Person.txt");
	std::vector<Person> to_marshal{ p1, p2 };

	if (marshal(to_marshal, &io))
	{
		auto from_unmarshal = unmarshal<PersonEx>(&io);
	}
#else
	std::string buf = p1.marshal();
	PersonEx pex = {};
	pex.unmarshal(buf);
	std::cout << pex.name << std::endl;
#endif
}

#endif

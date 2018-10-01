#ifndef __PROTOCOL_HPP_2017_04_06__
#define __PROTOCOL_HPP_2017_04_06__
#include <bas/memory.hpp>

#define MAKE_REQ(resp)	(resp & 0x7FFFFFFF)
#define MAKE_RESP(req)	(req  | 0x80000000)
#define IS_REQ(req)		((req & 0x80000000) == 0)
#define IS_RESP(resp)	((resp & 0x80000000) != 0)
static unsigned int magic_num = 0x12345678;

#pragma pack(1)

//	消息路由
struct jf_msg_router_t
{
	char src[32];						//	消息来源地
	char dst[32];						//	消息目的地

	void set_src(const char* src)
	{
		strcpy(this->src, src);
	}

	void set_dst(const char* dst)
	{
		strcpy(this->dst, dst);
	}

	void exchange()
	{
		char tmp_src[32] = {};
		strncpy(tmp_src, src, sizeof(tmp_src));

		mem_zero((void*)src, sizeof(src));
		strncpy(src, dst, sizeof(src));

		mem_zero((void*)dst, sizeof(dst));
		strncpy(dst, tmp_src, sizeof(dst));
	}
};

//	消息头部
struct jf_msg_header_t
{
public:
	jf_msg_router_t route;				//	消息路由
	unsigned int	cmdid;				//	命令号
	unsigned int	sid;				//	序列号
	unsigned int	ept_type;			//	加密类型
	unsigned int	header_len;			//	头部长度
	unsigned int	body_len;			//	消息体长度
	unsigned int	magic = magic_num;	//	消息验证字段

public:
	void serialize_to_string(std::string& str)
	{
		char bin[256] = {};
		mem_copy((void*)bin, (void*)this, sizeof(jf_msg_header_t));
		str = std::string(bin, sizeof(jf_msg_header_t));
	}
};

#pragma pack()

//	标准头
struct standard_header
{
	virtual int get_cmd_id()		= 0;
	virtual int get_sid()			= 0;
	virtual int get_ept_type()		= 0;
	virtual int get_header_len()	= 0;
	virtual int get_body_len()		= 0;
	virtual void release()			= 0;
	virtual standard_header* clone(const char*, int) = 0;
};

//	计费头部处理类，提供给底层模块（netport）使用
struct jf_header_t : standard_header
{
public:
	int get_cmd_id()		override	{ return header_.cmdid; }
	int get_sid()			override	{ return header_.sid; }
	int get_ept_type()		override	{ return header_.ept_type; }
	int get_header_len()	override	{ return sizeof(jf_msg_header_t); }
	int get_body_len()		override	{ return header_.body_len; }
	void release()			override	{ mem_delete_object(this); }
	standard_header* clone(const char* data, int len) override
	{
		jf_header_t* phdr = mem_create_object<jf_header_t>();
		mem_copy((void*)&phdr->header_, (void*)data, len);
		if (phdr->header_.magic != magic_num) { mem_delete_object(phdr); return 0; }
		return phdr;
	}

public:
	jf_msg_header_t header_;
};

#endif

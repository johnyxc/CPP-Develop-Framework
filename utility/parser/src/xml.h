#ifndef __XML_H_2015_08_20__
#define __XML_H_2015_08_20__
#include <map>
#include <string>
#include <utility/parser/3rd/inc/tinyxml.h>

typedef void* xml_node;

struct xml_t
{
public :
	xml_t();
	~xml_t();

public :
	int			decode_string(const char* context);
	char*		encode_string();
	xml_node	get_root();
	xml_node	new_root(const char* name, const char* ver, const char* enc, const char* standalone);
	int			get_int(xml_node node, int defval);
	__int64		get_int64(xml_node node, __int64 defval);
	double		get_double(xml_node node, double defval);
	bool		get_bool(xml_node node, bool defval);
	char*		get_string(xml_node node);
	int			get_attribute_int(xml_node node, const char* attrN, int defval);
	__int64		get_attribute_int64(xml_node node, const char* attrN, __int64 defval);
	double		get_attribute_double(xml_node node, const char* attrN, double defval);
	char*		get_attribute(xml_node node, const char* attrN);
	char*		get_all_attribute(xml_node node);
	void		get_all_attribute(xml_node node, std::map<std::string, std::string>& attrs);
	void		set_int(xml_node node, int v);
	void		set_int64(xml_node node, __int64 v);
	void		set_double(xml_node node, double v);
	void		set_bool(xml_node node, bool v);
	void		set_string(xml_node node, const char* v);
	void		set_attribute_int(xml_node node, const char* attrN, int v);
	void		set_attribute_int64(xml_node node, const char* attrN, __int64 v);
	void		set_attribute_double(xml_node node, const char* attrN, double v);
	void		set_attribute(xml_node node, const char* attrN, const char* v);
	void		remove_attribute(xml_node node, const char* attrN);
	xml_node	new_child(xml_node node, const char* nodeN, const char* v);
	bool		remove_child(xml_node node, xml_node child);
	xml_node	get_parent(xml_node node);
	const char* get_name(xml_node node);
	int			child_count(xml_node node);
	xml_node	first_child(xml_node node, const char* nodeN);
	xml_node	last_child(xml_node node, const char* nodeN);
	xml_node	prev_sibling(xml_node node, const char* nodeN);
	xml_node	next_sibling(xml_node node, const char* nodeN);
	xml_node	find_all(xml_node node, const char* nodeN, xml_node* nodeS, xml_node* recovR);
	void		mem_free(void* p);

private :
	TiXmlDocument* m_pDoc;
};
//////////////////////////////////////////////////////////////////////////

class CXml
{
public :
	CXml() : xml_impl_(), cur_node_(), fa_s_(), fa_r_() {}
	~CXml() {}

	CXml(const CXml& x)
	{
		*this = x;
	}

	CXml& operator = (const CXml& x)
	{
		if(this == &x) return *this;

		xml_impl_	= x.xml_impl_;
		cur_node_	= x.cur_node_;
		fa_s_		= x.fa_s_;
		fa_s_		= fa_r_;

		return *this;
	}

public :
	/**
	*	@brief			将内存中的数组转换成XML格式
	*	@param[in]		context: 内存中的字符串
	*	@return			返回0表示成功, 否则表示失败
	*/
	int decode(const char* context)
	{
		int ret = xml_impl_.decode_string(context);
		cur_node_ = xml_impl_.get_root();
		return ret;
	}

	/**
	*	@brief			将本对象中的XML字符串以C风格返回
	*	@return			成功返回C风格XML字符串(需调用mem_free释放), 否则返回0
	*/
	const char* encode()
	{
		return xml_impl_.encode_string();
	}

	/**
	*	@brief			新建XML根节点
	*	@param[in]		name: 根节点名称
	*	@param[in]		ver: XML版本号
	*	@param[in]		enc: XML编码方式
	*	@param[in]		standalone: 是否依赖外部文件, 一般情况下为空字符串
	*	@return			根节点对象
	*/
	CXml&	new_root(const char* name, const char* ver = "1.0", const char* enc = "utf-8", const char* standalone = "")
	{
		cur_node_ = xml_impl_.new_root(name, ver, enc, standalone);
		return *this;
	}

	/**
	*	@brief			获取XML根节点
	*	@return			根节点对象
	*/
	CXml&	get_root()
	{
		cur_node_ = xml_impl_.get_root();
		return *this;
	}

	/**
	*	@brief			获取整型节点的值
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			节点值
	*/
	int	get_int(int defval)
	{
		if(cur_node_)
		{
			return xml_impl_.get_int(cur_node_, defval);
		}
		return defval;
	}

	/**
	*	@brief			获取64位整型节点的值
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			64位节点值
	*/
	__int64	get_int64(__int64 defval)
	{
		if(cur_node_)
		{
			return xml_impl_.get_int64(cur_node_, defval);
		}
		return defval;
	}

	/**
	*	@brief			获取浮点型节点的值
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			节点值
	*/
	double get_double(double defval)
	{
		if(cur_node_)
		{
			return xml_impl_.get_double(cur_node_, defval);
		}
		return defval;
	}

	/**
	*	@brief			获取BOOL型节点的值
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			节点值
	*/
	bool get_bool(bool defval)
	{
		if(cur_node_)
		{
			return xml_impl_.get_bool(cur_node_, defval);
		}
		return defval;
	}

	/**
	*	@brief			获取节点的字符串值
	*	@return			成功返回C风格字符串(需调用mem_free释放), 否则返回0
	*/
	char*	get_string()
	{
		if(cur_node_)
		{
			return xml_impl_.get_string(cur_node_);
		}
		return 0;
	}

	/**
	*	@brief			获取整型属性值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			属性值
	*/
	int	get_attribute_int(const char* attrN, int defva)
	{
		if(cur_node_)
		{
			return xml_impl_.get_attribute_int(cur_node_, attrN, defva);
		}
		return 0;
	}

	/**
	*	@brief			获取64位整型属性值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			64位属性值
	*/
	__int64	get_attribute_int64(const char* attrN, __int64 defva)
	{
		if(cur_node_)
		{
			return xml_impl_.get_attribute_int64(cur_node_, attrN, defva);
		}
		return 0;
	}

	/**
	*	@brief			获取浮点型属性值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		defval: 接口执行失败时的默认值
	*	@return			浮点型属性值
	*/
	double get_attribute_double(const char* attrN, double defva)
	{
		if(cur_node_)
		{
			return xml_impl_.get_attribute_double(cur_node_, attrN, defva);
		}
		return 0;
	}

	/**
	*	@brief			获取属性字符串
	*	@param[in]		attrN: 属性名称
	*	@return			成功返回属性字符串(需调用mem_free释放), 否则返回0
	*/
	char*	get_attribute(const char* attrN)
	{
		if(cur_node_)
		{
			return xml_impl_.get_attribute(cur_node_, attrN);
		}
		return 0;
	}

	/**
	*	@brief			获取节点所有属性值
	*	@return			成功返回属性字符串(需调用mem_free释放), 字符串
	*	以'/'分割, 名称在前值在后, 接口失败返回0
	*/
	char*	get_all_attribute()
	{
		if(cur_node_)
		{
			return xml_impl_.get_all_attribute(cur_node_);
		}
		return 0;
	}

	void	get_all_attribute(std::map<std::string, std::string>& attrs)
	{
		if (cur_node_)
		{
			return xml_impl_.get_all_attribute(cur_node_, attrs);
		}
	}

	/**
	*	@brief			设置节点整型值
	*	@param[in]		v: 整型值
	*/
	CXml&	set_int(int v)
	{
		if(cur_node_)
		{
			xml_impl_.set_int(cur_node_, v);
		}
		return *this;
	}

	/**
	*	@brief			设置节点64位整型值
	*	@param[in]		v: 64位整型值
	*/
	CXml&	set_int64(__int64 v)
	{
		if(cur_node_)
		{
			xml_impl_.set_int64(cur_node_, v);
		}
		return *this;
	}

	/**
	*	@brief			设置节点浮点型值
	*	@param[in]		v: 浮点型值
	*/
	CXml&	set_double(double v)
	{
		if(cur_node_)
		{
			xml_impl_.set_double(cur_node_, v);
		}
		return *this;
	}

	/**
	*	@brief			设置节点BOOL型值
	*	@param[in]		v: BOOL型值
	*/
	CXml&	set_bool(bool v)
	{
		if(cur_node_)
		{
			xml_impl_.set_bool(cur_node_, v);
		}
		return *this;
	}

	/**
	*	@brief			设置节点字符串值
	*	@param[in]		v: C风格字符串值
	*/
	CXml&	set_string(const char* v)
	{
		if(cur_node_)
		{
			xml_impl_.set_string(cur_node_, v);
		}
		return *this;
	}

	/**
	*	@brief			设置属性整型值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		v: 整型值
	*/
	CXml&	set_attribute_int(const char* attrN, int v)
	{
		if(cur_node_)
		{
			xml_impl_.set_attribute_int(cur_node_, attrN, v);
		}
		return *this;
	}

	/**
	*	@brief			设置属性64位整型值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		v: 64位整型值
	*/
	CXml&	set_attribute_int64(const char* attrN, __int64 v)
	{
		if(cur_node_)
		{
			xml_impl_.set_attribute_int64(cur_node_, attrN, v);
		}
		return *this;
	}

	/**
	*	@brief			设置属性浮点型值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		v: 浮点型值
	*/
	CXml&	set_attribute_double(const char* attrN, double v)
	{
		if(cur_node_)
		{
			xml_impl_.set_attribute_double(cur_node_, attrN, v);
		}
		return *this;
	}

	/**
	*	@brief			设置属性字符串值
	*	@param[in]		attrN: 属性名称
	*	@param[in]		v: C风格字符串值
	*/
	CXml&	set_attribute(const char* attrN, const char* v)
	{
		if(cur_node_)
		{
			xml_impl_.set_attribute(cur_node_, attrN, v);
		}
		return *this;
	}

	/**
	*	@brief			在指定节点下创建新节点
	*	@param[in]		nodeN: 新节点名称
	*	@param[in]		v: 新节点字符串值, 传0代表在此不予赋值
	*/
	CXml&	new_child(const char* nodeN, const char* v)
	{
		if(cur_node_)
		{
			cur_node_ = xml_impl_.new_child(cur_node_, nodeN, v);
		}
		return *this;
	}

	/**
	*	@brief			删除指定节点
	*/
	bool	remove_node()
	{
		if(cur_node_)
		{
			xml_node pa = xml_impl_.get_parent(cur_node_);
			return xml_impl_.remove_child(pa, cur_node_);
		}
		return false;
	}

	/**
	*	@brief			获取当前节点父节点句柄
	*	@param[in]		node: 节点句柄
	*/
	CXml&	get_parent()
	{
		if(cur_node_)
		{
			cur_node_ = xml_impl_.get_parent(cur_node_);
		}
		return *this;
	}

	/**
	*	@brief			获取当前节点名称
	*	@return			C风格节点名称字符串
	*/
	const char* get_name()
	{
		if(cur_node_)
		{
			return xml_impl_.get_name(cur_node_);
		}
		return 0;
	}

	/**
	*	@brief			获取当前节点子节点数
	*	@return			子节点数
	*/
	int	child_count()
	{
		if(cur_node_)
		{
			return xml_impl_.child_count(cur_node_);
		}
		return 0;
	}

	/**
	*	@brief			查找指定节点下第一个具名或不具名子节点
	*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
	*	@return			成功返回true, 否则返回false
	*/
	bool first_child(const char* nodeN = 0)
	{
		if(cur_node_)
		{
			xml_node tmp = xml_impl_.first_child(cur_node_, nodeN);
			if(tmp)
			{
				cur_node_ = tmp;
				return true;
			}
		}
		return false;
	}

	/**
	*	@brief			查找指定节点下最后一个具名或不具名子节点
	*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
	*	@return			成功返回true, 否则返回false
	*/
	bool last_child(const char* nodeN = 0)
	{
		if(cur_node_)
		{
			xml_node tmp = xml_impl_.last_child(cur_node_, nodeN);
			if(tmp)
			{
				cur_node_ = tmp;
				return true;
			}
		}
		return false;
	}

	/**
	*	@brief			获取指定节点上一个具名或不具名兄弟节点
	*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
	*	@return			成功返回true, 否则返回false
	*/
	bool prev_sibling(const char* nodeN = 0)
	{
		if(cur_node_)
		{
			xml_node tmp = xml_impl_.prev_sibling(cur_node_, nodeN);
			if(tmp)
			{
				cur_node_ = tmp;
				return true;
			}
		}
		return false;
	}

	/**
	*	@brief			获取指定节点下一个具名或不具名兄弟节点
	*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
	*	@return			成功返回true, 否则返回false
	*/
	bool next_sibling(const char* nodeN = 0)
	{
		if(cur_node_)
		{
			xml_node tmp = xml_impl_.next_sibling(cur_node_, nodeN);
			if(tmp)
			{
				cur_node_ = tmp;
				return true;
			}
		}
		return false;
	}

	/**
	*	@brief			查找一个节点下所有指定名称的节点
	*	@param[in]		nodeN: 欲查找的节点名称, 不允许传0
	*	@return			成功返回true, 否则返回false
	*	@remark			循环调用此接口, 每次返回一个给定名称的节点句柄, 直到返回0代表查找结束
	*/
	bool find_all(const char* nodeN)
	{
		if(cur_node_)
		{
			xml_node tmp = xml_impl_.find_all(cur_node_, nodeN, &fa_s_, &fa_r_);
			if(tmp)
			{
				cur_node_ = tmp;
				return true;
			}
		}
		return false;
	}

	/**
	*	@brief			判断XML对象是否有效
	*	@return			有效返回true, 否则返回false
	*/
	bool valid()
	{
		return cur_node_ ? true : false;
	}

	void mem_free(void* p)
	{
		xml_impl_.mem_free(p);
	}

private :
	xml_t		xml_impl_;
	xml_node	cur_node_;
	xml_node	fa_s_;
	xml_node	fa_r_;
};

#endif

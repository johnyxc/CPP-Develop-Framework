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
	*	@brief			���ڴ��е�����ת����XML��ʽ
	*	@param[in]		context: �ڴ��е��ַ���
	*	@return			����0��ʾ�ɹ�, �����ʾʧ��
	*/
	int decode(const char* context)
	{
		int ret = xml_impl_.decode_string(context);
		cur_node_ = xml_impl_.get_root();
		return ret;
	}

	/**
	*	@brief			���������е�XML�ַ�����C��񷵻�
	*	@return			�ɹ�����C���XML�ַ���(�����mem_free�ͷ�), ���򷵻�0
	*/
	const char* encode()
	{
		return xml_impl_.encode_string();
	}

	/**
	*	@brief			�½�XML���ڵ�
	*	@param[in]		name: ���ڵ�����
	*	@param[in]		ver: XML�汾��
	*	@param[in]		enc: XML���뷽ʽ
	*	@param[in]		standalone: �Ƿ������ⲿ�ļ�, һ�������Ϊ���ַ���
	*	@return			���ڵ����
	*/
	CXml&	new_root(const char* name, const char* ver = "1.0", const char* enc = "utf-8", const char* standalone = "")
	{
		cur_node_ = xml_impl_.new_root(name, ver, enc, standalone);
		return *this;
	}

	/**
	*	@brief			��ȡXML���ڵ�
	*	@return			���ڵ����
	*/
	CXml&	get_root()
	{
		cur_node_ = xml_impl_.get_root();
		return *this;
	}

	/**
	*	@brief			��ȡ���ͽڵ��ֵ
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			�ڵ�ֵ
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
	*	@brief			��ȡ64λ���ͽڵ��ֵ
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			64λ�ڵ�ֵ
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
	*	@brief			��ȡ�����ͽڵ��ֵ
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			�ڵ�ֵ
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
	*	@brief			��ȡBOOL�ͽڵ��ֵ
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			�ڵ�ֵ
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
	*	@brief			��ȡ�ڵ���ַ���ֵ
	*	@return			�ɹ�����C����ַ���(�����mem_free�ͷ�), ���򷵻�0
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
	*	@brief			��ȡ��������ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			����ֵ
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
	*	@brief			��ȡ64λ��������ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			64λ����ֵ
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
	*	@brief			��ȡ����������ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
	*	@return			����������ֵ
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
	*	@brief			��ȡ�����ַ���
	*	@param[in]		attrN: ��������
	*	@return			�ɹ����������ַ���(�����mem_free�ͷ�), ���򷵻�0
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
	*	@brief			��ȡ�ڵ���������ֵ
	*	@return			�ɹ����������ַ���(�����mem_free�ͷ�), �ַ���
	*	��'/'�ָ�, ������ǰֵ�ں�, �ӿ�ʧ�ܷ���0
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
	*	@brief			���ýڵ�����ֵ
	*	@param[in]		v: ����ֵ
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
	*	@brief			���ýڵ�64λ����ֵ
	*	@param[in]		v: 64λ����ֵ
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
	*	@brief			���ýڵ㸡����ֵ
	*	@param[in]		v: ������ֵ
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
	*	@brief			���ýڵ�BOOL��ֵ
	*	@param[in]		v: BOOL��ֵ
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
	*	@brief			���ýڵ��ַ���ֵ
	*	@param[in]		v: C����ַ���ֵ
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
	*	@brief			������������ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		v: ����ֵ
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
	*	@brief			��������64λ����ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		v: 64λ����ֵ
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
	*	@brief			�������Ը�����ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		v: ������ֵ
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
	*	@brief			���������ַ���ֵ
	*	@param[in]		attrN: ��������
	*	@param[in]		v: C����ַ���ֵ
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
	*	@brief			��ָ���ڵ��´����½ڵ�
	*	@param[in]		nodeN: �½ڵ�����
	*	@param[in]		v: �½ڵ��ַ���ֵ, ��0�����ڴ˲��踳ֵ
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
	*	@brief			ɾ��ָ���ڵ�
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
	*	@brief			��ȡ��ǰ�ڵ㸸�ڵ���
	*	@param[in]		node: �ڵ���
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
	*	@brief			��ȡ��ǰ�ڵ�����
	*	@return			C���ڵ������ַ���
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
	*	@brief			��ȡ��ǰ�ڵ��ӽڵ���
	*	@return			�ӽڵ���
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
	*	@brief			����ָ���ڵ��µ�һ�������򲻾����ӽڵ�
	*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
	*	@return			�ɹ�����true, ���򷵻�false
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
	*	@brief			����ָ���ڵ������һ�������򲻾����ӽڵ�
	*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
	*	@return			�ɹ�����true, ���򷵻�false
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
	*	@brief			��ȡָ���ڵ���һ�������򲻾����ֵܽڵ�
	*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
	*	@return			�ɹ�����true, ���򷵻�false
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
	*	@brief			��ȡָ���ڵ���һ�������򲻾����ֵܽڵ�
	*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
	*	@return			�ɹ�����true, ���򷵻�false
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
	*	@brief			����һ���ڵ�������ָ�����ƵĽڵ�
	*	@param[in]		nodeN: �����ҵĽڵ�����, ������0
	*	@return			�ɹ�����true, ���򷵻�false
	*	@remark			ѭ�����ô˽ӿ�, ÿ�η���һ���������ƵĽڵ���, ֱ������0������ҽ���
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
	*	@brief			�ж�XML�����Ƿ���Ч
	*	@return			��Ч����true, ���򷵻�false
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

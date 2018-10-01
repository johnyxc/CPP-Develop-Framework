#include "stdafx.h"
#include <bas/memory.hpp>
#include <utility/parser/3rd/inc/xpath_static.h>
#include <utility/parser/src/xml.h>
#define itoa _itoa
//////////////////////////////////////////////////////////////////////////

xml_t::xml_t() : m_pDoc()
{
	m_pDoc = new TiXmlDocument;
}

xml_t::~xml_t()
{
	if(m_pDoc) delete m_pDoc;
}

/**
*	@brief			将内存中的数组转换成XML格式
*	@param[in]		context: 内存中的字符串
*	@return			返回0表示成功, 否则表示失败
*/
int xml_t::decode_string(const char* context)
{
	if(!m_pDoc || !context) return -1;
	m_pDoc->Clear();
	if(!(m_pDoc->Parse(context)))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
*	@brief			将本对象中的XML字符串以C风格返回
*	@return			成功返回C风格XML字符串(需调用mem_free释放), 否则返回0
*/
char* xml_t::encode_string()
{
	if(!m_pDoc) return 0;
	TiXmlPrinter printer;

	m_pDoc->Accept(&printer);
	const char* psrc = 0;
	psrc = printer.CStr();
	if(!psrc) return 0;
	int len = strlen(psrc);
	if(len == 0) return 0;

	//	分配存放返回值的缓冲区，需由使用者释放
	char* pdst = (char*)mem_zalloc(len + 1);
	if(!pdst) return 0;
	mem_copy(pdst, psrc, len);

	return pdst;
}

/**
*	@brief			获取XML根节点
*	@return			根节点句柄
*/
xml_node xml_t::get_root()
{
	if(!m_pDoc) return 0;
	TiXmlNode* pNode = m_pDoc->RootElement();
	return (xml_node)pNode;
}

/**
*	@brief			新建XML根节点
*	@param[in]		name: 根节点名称
*	@param[in]		ver: XML版本号
*	@param[in]		enc: XML编码方式
*	@param[in]		standalone: 是否依赖外部文件, 一般情况下为空字符串
*	@return			根节点句柄
*/
xml_node xml_t::new_root(const char* name, const char* ver, const char* enc, const char* standalone)
{
	if(!m_pDoc || !name || !ver || !enc || !standalone) return 0;
	m_pDoc->Clear();
	TiXmlDeclaration* decl = new TiXmlDeclaration(ver, enc, standalone);//_xml_create_TiXmlDeclaration(ver, enc, standalone);
	m_pDoc->LinkEndChild(decl);
	TiXmlElement* pEle = new TiXmlElement(name);//_xml_create_TiXmlElement(name);
	m_pDoc->LinkEndChild(pEle);
	return (xml_node)pEle;
}

/**
*	@brief			获取整型节点的值
*	@param[in]		node: 节点句柄
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			节点值
*/
int xml_t::get_int(xml_node node, int defval)
{
	char* str = get_string(node);
	if(str)
	{
		int retv = atoi(str);
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取64位整型节点的值
*	@param[in]		node: 节点句柄
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			64位节点值
*/
__int64 xml_t::get_int64(xml_node node, __int64 defval)
{
	char* str = get_string(node);
	if(str)
	{
		__int64 retv;
#ifdef WIN32
		retv = _atoi64(str);
#else
		retv = atoll(str);
#endif
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取浮点型节点的值
*	@param[in]		node: 节点句柄
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			节点值
*/
double xml_t::get_double(xml_node node, double defval)
{
	char* str = get_string(node);
	if(str)
	{
		double retv = atof(str);
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取BOOL型节点的值
*	@param[in]		node: 节点句柄
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			节点值
*/
bool xml_t::get_bool(xml_node node, bool defval)
{
	char* str = get_string(node);
	if(str)
	{
		char* tl = str;
		bool retv = (!strcmp(str, "false") || !strcmp(str, "FALSE")) ? false : true;
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取节点的字符串值
*	@param[in]		node: 节点句柄
*	@return			成功返回C风格字符串(需调用mem_free释放), 否则返回0
*/
char* xml_t::get_string(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if(pNode->Type() != TiXmlNode::ELEMENT) return 0;
	TiXmlElement* pEle = pNode->ToElement();
	if(pEle)
	{
		const char* psrc = pEle->GetText();
		if(!psrc) return 0;
		int len = strlen(psrc);
		if(len == 0) return 0;

		//	返回缓冲区需由使用者释放
		if((psrc[0] == '\"') && (psrc[len - 1] == '\"'))
		{
			char* pdst_r = (char*)mem_zalloc(len - 1);
			mem_copy(pdst_r, psrc + 1, len - 2);
			return pdst_r;
		}

		char* pdst = (char*)mem_zalloc(len + 1);
		mem_copy(pdst, psrc, len);
		return pdst;
	}
	else
	{
		return 0;
	}
}

/**
*	@brief			获取整型属性值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			属性值
*/
int xml_t::get_attribute_int(xml_node node, const char* attrN, int defval)
{
	char* str = get_attribute(node, attrN);
	if(str)
	{
		int retv = atoi(str);
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取64位整型属性值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			64位属性值
*/
__int64 xml_t::get_attribute_int64(xml_node node, const char* attrN, __int64 defval)
{
	char* str = get_attribute(node, attrN);
	if(str)
	{
		__int64 retv;
#ifdef _WIN32
		retv = _atoi64(str);
#else
		retv = atoll(str);
#endif
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取浮点型属性值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		defval: 接口执行失败时的默认值
*	@return			浮点型属性值
*/
double xml_t::get_attribute_double(xml_node node, const char* attrN, double defval)
{
	char* str = get_attribute(node, attrN);
	if(str)
	{
		double retv = atof(str);
		mem_free(str);
		return retv;
	}
	else
	{
		return defval;
	}
}

/**
*	@brief			获取属性字符串
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@return			成功返回属性字符串(需调用mem_free释放), 否则返回0
*/
char* xml_t::get_attribute(xml_node node, const char* attrN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !attrN) return 0;
	TiXmlElement* pEle = pNode->ToElement();
	if(pEle)
	{
		const char* attr = pEle->Attribute(attrN);
		if(!attr) return 0;
		int len  = strlen(attr);
		if(len == 0) return 0;
		char* rets = (char*)mem_zalloc(len + 1);
		mem_copy(rets, attr, len);
		return rets;
	}
	else
	{
		return 0;
	}
}

/**
*	@brief			获取节点所有属性值
*	@param[in]		node: 节点句柄
*	@return			成功返回属性字符串(需调用mem_free释放), 字符串
*	以'/'分割, 名称在前值在后, 接口失败返回0
*/
char* xml_t::get_all_attribute(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if(pNode->Type() != TiXmlNode::ELEMENT) return 0;
	TiXmlElement* pEle = pNode->ToElement();
	if(pEle)
	{
		char* rets = (char*)mem_zalloc(1024);
		char* head = rets;
		const char* ws = " ";
		TiXmlAttribute* pAttr = pEle->FirstAttribute();
		while(pAttr)
		{
			const char* name = pAttr->Name();
			const char* attr = pAttr->Value();
			int nl = strlen(name);
			int al = strlen(attr);
			{
				mem_copy(rets, name, nl);
				rets += nl;
				mem_copy(rets, "/", 1);
				rets += 1;
				mem_copy(rets, al ? attr : ws, al ? al : 1);
				(rets += al, al) || (rets += 1);	//	rets前进一个字符串或一个空格的长度
				mem_copy(rets, "/", 1);
				rets += 1;
			}
			pAttr = pAttr->Next();
		}
		return head;
	}
	else
	{
		return 0;
	}
}

void xml_t::get_all_attribute(xml_node node, std::map<std::string, std::string>& attrs)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if (pNode->Type() != TiXmlNode::ELEMENT) return;
	TiXmlElement* pEle = pNode->ToElement();
	if (pEle)
	{
		TiXmlAttribute* pAttr = pEle->FirstAttribute();
		while (pAttr)
		{
			const char* name = pAttr->Name();
			const char* attr = pAttr->Value();
			if (name && attr) attrs[name] = attr;
			pAttr = pAttr->Next();
		}
	}
}

/**
*	@brief			设置节点整型值
*	@param[in]		node: 节点句柄
*	@param[in]		v: 整型值
*/
void xml_t::set_int(xml_node node, int v)
{
	char buf[15] = {};
	itoa(v, buf, 10);
	set_string(node, buf);
}

/**
*	@brief			设置节点64位整型值
*	@param[in]		node: 节点句柄
*	@param[in]		v: 64位整型值
*/
void xml_t::set_int64(xml_node node, __int64 v)
{
	char buf[15] = {};
	_i64toa(v, buf, 10);
	set_string(node, buf);
}

/**
*	@brief			设置节点浮点型值
*	@param[in]		node: 节点句柄
*	@param[in]		v: 浮点型值
*/
void xml_t::set_double(xml_node node, double v)
{
	char buf[30] = {};
	sprintf(buf, "%lf", v);
	set_string(node, buf);
}

/**
*	@brief			设置节点BOOL型值
*	@param[in]		node: 节点句柄
*	@param[in]		v: BOOL型值
*/
void xml_t::set_bool(xml_node node, bool v)
{
	//set_int(node, v);
	char buf[15] = {};
	v ? strcpy(buf, "true") : strcpy(buf, "false");
	set_string(node, buf);
}

/**
*	@brief			设置节点字符串值
*	@param[in]		node: 节点句柄
*	@param[in]		v: C风格字符串值
*/
void xml_t::set_string(xml_node node, const char* v)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !v) return;
	TiXmlElement* pEle = pNode->ToElement();
	TiXmlText* pTxt = new TiXmlText(v);//_xml_create_TiXmlText(v);
	pEle->LinkEndChild(pTxt);
}

/**
*	@brief			设置属性整型值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		v: 整型值
*/
void xml_t::set_attribute_int(xml_node node, const char* attrN, int v)
{
	char buf[15] = {};
	itoa(v, buf, 10);
	set_attribute(node,attrN, buf);
}

/**
*	@brief			设置属性64位整型值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		v: 64位整型值
*/
void xml_t::set_attribute_int64(xml_node node, const char* attrN, __int64 v)
{
	char buf[25] = {};
	_i64toa(v, buf, 10);
	set_attribute(node,attrN, buf);
}

/**
*	@brief			设置属性浮点型值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		v: 浮点型值
*/
void xml_t::set_attribute_double(xml_node node, const char* attrN, double v)
{
	char buf[30] = {};
	sprintf(buf, "%lf", v);
	set_attribute(node, attrN, buf);
}

/**
*	@brief			设置属性字符串值
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*	@param[in]		v: C风格字符串值
*/
void xml_t::set_attribute(xml_node node, const char* attrN, const char* v)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !attrN || !v) return;
	TiXmlElement* pEle = pNode->ToElement();
	pEle->SetAttribute(attrN, v);
}

/**
*	@brief			删除节点指定的属性
*	@param[in]		node: 节点句柄
*	@param[in]		attrN: 属性名称
*/
void xml_t::remove_attribute(xml_node node, const char* attrN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !attrN) return;
	TiXmlElement* pEle = pNode->ToElement();
	pEle->RemoveAttribute(attrN);
}

/**
*	@brief			在指定节点下创建新节点
*	@param[in]		node: 节点句柄
*	@param[in]		nodeN: 新节点名称
*	@param[in]		v: 新节点字符串值, 传0代表在此不予赋值
*/
xml_node xml_t::new_child(xml_node node, const char* nodeN, const char* v)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !nodeN) return 0;
	TiXmlElement* pEle = new TiXmlElement(nodeN);//_xml_create_TiXmlElement(nodeN);
	if(v)
	{
		TiXmlText* pTxt = new TiXmlText(v);//_xml_create_TiXmlText(v);
		pEle->LinkEndChild(pTxt);
	}
	pNode->LinkEndChild(pEle);
	return (xml_node)pEle;
}

/**
*	@brief			删除指定节点
*	@param[in]		node: 节点句柄
*	@param[in]		child: 子节点句柄
*/
bool xml_t::remove_child(xml_node node, xml_node child)
{
	if(!node || !child) return false;
	TiXmlNode* pNode = (TiXmlNode*)node;
	if(!pNode) return false;
	return pNode->RemoveChild((TiXmlNode*)child);
}

/**
*	@brief			获取当前节点父节点句柄
*	@param[in]		node: 节点句柄
*/
xml_node xml_t::get_parent(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	return (xml_node)pNode->Parent();
}

/**
*	@brief			获取当前节点名称
*	@param[in]		node: 节点句柄
*	@return			C风格节点名称字符串
*/
const char* xml_t::get_name(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	return pNode->Value();
}

/**
*	@brief			获取当前节点子节点数
*	@param[in]		node: 节点句柄
*	@return			子节点数
*/
int xml_t::child_count(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return -1;
	TiXmlNode* pf = pNode->FirstChild();
	int cnt = 0;
	while(pf)
	{
		++cnt;
		pf = pf->NextSibling();
	}
	return cnt;
}

/**
*	@brief			查找指定节点下第一个具名或不具名子节点
*	@param[in]		node: 欲查找节点的父节点
*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
*	@return			成功返回查找到的节点句柄, 否则返回0
*/
xml_node xml_t::first_child(xml_node node, const char* nodeN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	if(nodeN)
	{
		return (xml_node)(pNode->FirstChild(nodeN));
	}
	else
	{
		return (xml_node)(pNode->FirstChild());
	}
}

/**
*	@brief			查找指定节点下最后一个具名或不具名子节点
*	@param[in]		node: 欲查找节点的父节点
*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
*	@return			成功返回查找到的节点句柄, 否则返回0
*/
xml_node xml_t::last_child(xml_node node, const char* nodeN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	if(nodeN)
	{
		return (xml_node)(pNode->LastChild(nodeN));
	}
	else
	{
		return (xml_node)(pNode->LastChild());
	}
}

/**
*	@brief			获取指定节点上一个具名或不具名兄弟节点
*	@param[in]		node: 节点句柄
*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
*	@return			成功返回查找到的节点句柄, 否则返回0
*/
xml_node xml_t::prev_sibling(xml_node node, const char* nodeN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	if(nodeN)
	{
		return (xml_node)(pNode->PreviousSibling(nodeN));
	}
	else
	{
		return (xml_node)(pNode->PreviousSibling());
	}
}

/**
*	@brief			获取指定节点下一个具名或不具名兄弟节点
*	@param[in]		node: 节点句柄
*	@param[in]		nodeN: 欲查找的节点名称, 传0表示不查找具名节点
*	@return			成功返回查找到的节点句柄, 否则返回0
*/
xml_node xml_t::next_sibling(xml_node node, const char* nodeN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	if(nodeN)
	{
		return (xml_node)(pNode->NextSibling(nodeN));
	}
	else
	{
		return (xml_node)(pNode->NextSibling());
	}
}

/**
*	@brief			查找一个节点下所有指定名称的节点
*	@param[in]		node: 欲查找节点的父节点句柄
*	@param[in]		nodeN: 欲查找的节点名称, 不允许传0
*	@param[in]		nodeS: 内部使用, 外部无需关心
*	@param[in]		recovR: 内部使用, 外部无需关心
*	@return			成功返回查找到的节点句柄, 否则返回0
*	@remark			循环调用此接口, 每次返回一个给定名称的节点句柄, 直到返回0代表查找结束
*/
xml_node xml_t::find_all(xml_node node, const char* nodeN, xml_node* nodeS, xml_node* recovR)
{
	if(!nodeN || !nodeS || !recovR) return 0;	//	只提供具名搜索

	TiXmlNode** pNodeS = (TiXmlNode**)nodeS;
	TiXmlNode** pRecov = (TiXmlNode**)recovR;
	TiXmlNode*  iter   = 0;
	TiXmlNode*  pTemp  = 0;

	if(!(*pNodeS))	//	记录迭代起始点
	{
		iter = *pNodeS = (TiXmlNode*)node;
		if((*pNodeS)->FirstChild()->Type() == TiXmlNode::TEXT) return 0;	//	禁止从非根节点开始迭代
		pTemp = (*pNodeS)->FirstChild();
		if(!pTemp) return 0;
	}
	else
	{
		if(*pRecov)
		{
			pTemp = iter = *pRecov;
			*pRecov = 0;
		}
		else
		{
			iter = (TiXmlNode*)node;
			pTemp = iter->NextSibling();
		}
	}

	for(;;)	//	循环查找具名节点
	{
		if(pTemp)
		{
			iter = pTemp;
			TiXmlNode* px = pTemp->FirstChild();
			if(!px)
			{
				if(!strcmp(nodeN, iter->Value()))
				{
					break;
				}
				else
				{
					pTemp = pTemp->NextSibling();
					continue;
				}
			}
			if(px->Type() == TiXmlNode::TEXT)
			{
				if(!strcmp(nodeN, iter->Value()))
				{
					break;
				}
				else
				{
					pTemp = pTemp->NextSibling();
					continue;
				}
			}
			else
			{
				if(!strcmp(nodeN, iter->Value()))
				{
					*pRecov = pTemp->FirstChild();
					iter = pTemp;
					break;
				}
				else
				{
					pTemp = pTemp->FirstChild();
					continue;
				}
			}
		}
		else
		{
			iter = pTemp = iter->Parent();
			if(iter == *pNodeS)
			{
				*pNodeS = 0;
				*pRecov = 0;
				return 0;
			}
			pTemp = pTemp->NextSibling();
			continue;
		}
	}

	return (xml_node)iter;
}

void xml_t::mem_free(void* p)
{
	::mem_free(p);
}

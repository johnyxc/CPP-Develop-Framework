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
*	@brief			���ڴ��е�����ת����XML��ʽ
*	@param[in]		context: �ڴ��е��ַ���
*	@return			����0��ʾ�ɹ�, �����ʾʧ��
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
*	@brief			���������е�XML�ַ�����C��񷵻�
*	@return			�ɹ�����C���XML�ַ���(�����mem_free�ͷ�), ���򷵻�0
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

	//	�����ŷ���ֵ�Ļ�����������ʹ�����ͷ�
	char* pdst = (char*)mem_zalloc(len + 1);
	if(!pdst) return 0;
	mem_copy(pdst, psrc, len);

	return pdst;
}

/**
*	@brief			��ȡXML���ڵ�
*	@return			���ڵ���
*/
xml_node xml_t::get_root()
{
	if(!m_pDoc) return 0;
	TiXmlNode* pNode = m_pDoc->RootElement();
	return (xml_node)pNode;
}

/**
*	@brief			�½�XML���ڵ�
*	@param[in]		name: ���ڵ�����
*	@param[in]		ver: XML�汾��
*	@param[in]		enc: XML���뷽ʽ
*	@param[in]		standalone: �Ƿ������ⲿ�ļ�, һ�������Ϊ���ַ���
*	@return			���ڵ���
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
*	@brief			��ȡ���ͽڵ��ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			�ڵ�ֵ
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
*	@brief			��ȡ64λ���ͽڵ��ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			64λ�ڵ�ֵ
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
*	@brief			��ȡ�����ͽڵ��ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			�ڵ�ֵ
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
*	@brief			��ȡBOOL�ͽڵ��ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			�ڵ�ֵ
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
*	@brief			��ȡ�ڵ���ַ���ֵ
*	@param[in]		node: �ڵ���
*	@return			�ɹ�����C����ַ���(�����mem_free�ͷ�), ���򷵻�0
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

		//	���ػ���������ʹ�����ͷ�
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
*	@brief			��ȡ��������ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			����ֵ
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
*	@brief			��ȡ64λ��������ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			64λ����ֵ
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
*	@brief			��ȡ����������ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		defval: �ӿ�ִ��ʧ��ʱ��Ĭ��ֵ
*	@return			����������ֵ
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
*	@brief			��ȡ�����ַ���
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@return			�ɹ����������ַ���(�����mem_free�ͷ�), ���򷵻�0
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
*	@brief			��ȡ�ڵ���������ֵ
*	@param[in]		node: �ڵ���
*	@return			�ɹ����������ַ���(�����mem_free�ͷ�), �ַ���
*	��'/'�ָ�, ������ǰֵ�ں�, �ӿ�ʧ�ܷ���0
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
				(rets += al, al) || (rets += 1);	//	retsǰ��һ���ַ�����һ���ո�ĳ���
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
*	@brief			���ýڵ�����ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		v: ����ֵ
*/
void xml_t::set_int(xml_node node, int v)
{
	char buf[15] = {};
	itoa(v, buf, 10);
	set_string(node, buf);
}

/**
*	@brief			���ýڵ�64λ����ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		v: 64λ����ֵ
*/
void xml_t::set_int64(xml_node node, __int64 v)
{
	char buf[15] = {};
	_i64toa(v, buf, 10);
	set_string(node, buf);
}

/**
*	@brief			���ýڵ㸡����ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		v: ������ֵ
*/
void xml_t::set_double(xml_node node, double v)
{
	char buf[30] = {};
	sprintf(buf, "%lf", v);
	set_string(node, buf);
}

/**
*	@brief			���ýڵ�BOOL��ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		v: BOOL��ֵ
*/
void xml_t::set_bool(xml_node node, bool v)
{
	//set_int(node, v);
	char buf[15] = {};
	v ? strcpy(buf, "true") : strcpy(buf, "false");
	set_string(node, buf);
}

/**
*	@brief			���ýڵ��ַ���ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		v: C����ַ���ֵ
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
*	@brief			������������ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		v: ����ֵ
*/
void xml_t::set_attribute_int(xml_node node, const char* attrN, int v)
{
	char buf[15] = {};
	itoa(v, buf, 10);
	set_attribute(node,attrN, buf);
}

/**
*	@brief			��������64λ����ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		v: 64λ����ֵ
*/
void xml_t::set_attribute_int64(xml_node node, const char* attrN, __int64 v)
{
	char buf[25] = {};
	_i64toa(v, buf, 10);
	set_attribute(node,attrN, buf);
}

/**
*	@brief			�������Ը�����ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		v: ������ֵ
*/
void xml_t::set_attribute_double(xml_node node, const char* attrN, double v)
{
	char buf[30] = {};
	sprintf(buf, "%lf", v);
	set_attribute(node, attrN, buf);
}

/**
*	@brief			���������ַ���ֵ
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*	@param[in]		v: C����ַ���ֵ
*/
void xml_t::set_attribute(xml_node node, const char* attrN, const char* v)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !attrN || !v) return;
	TiXmlElement* pEle = pNode->ToElement();
	pEle->SetAttribute(attrN, v);
}

/**
*	@brief			ɾ���ڵ�ָ��������
*	@param[in]		node: �ڵ���
*	@param[in]		attrN: ��������
*/
void xml_t::remove_attribute(xml_node node, const char* attrN)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT) || !attrN) return;
	TiXmlElement* pEle = pNode->ToElement();
	pEle->RemoveAttribute(attrN);
}

/**
*	@brief			��ָ���ڵ��´����½ڵ�
*	@param[in]		node: �ڵ���
*	@param[in]		nodeN: �½ڵ�����
*	@param[in]		v: �½ڵ��ַ���ֵ, ��0�����ڴ˲��踳ֵ
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
*	@brief			ɾ��ָ���ڵ�
*	@param[in]		node: �ڵ���
*	@param[in]		child: �ӽڵ���
*/
bool xml_t::remove_child(xml_node node, xml_node child)
{
	if(!node || !child) return false;
	TiXmlNode* pNode = (TiXmlNode*)node;
	if(!pNode) return false;
	return pNode->RemoveChild((TiXmlNode*)child);
}

/**
*	@brief			��ȡ��ǰ�ڵ㸸�ڵ���
*	@param[in]		node: �ڵ���
*/
xml_node xml_t::get_parent(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	if((pNode->Type() != TiXmlNode::ELEMENT)) return 0;
	return (xml_node)pNode->Parent();
}

/**
*	@brief			��ȡ��ǰ�ڵ�����
*	@param[in]		node: �ڵ���
*	@return			C���ڵ������ַ���
*/
const char* xml_t::get_name(xml_node node)
{
	TiXmlNode* pNode = (TiXmlNode*)node;
	return pNode->Value();
}

/**
*	@brief			��ȡ��ǰ�ڵ��ӽڵ���
*	@param[in]		node: �ڵ���
*	@return			�ӽڵ���
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
*	@brief			����ָ���ڵ��µ�һ�������򲻾����ӽڵ�
*	@param[in]		node: �����ҽڵ�ĸ��ڵ�
*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
*	@return			�ɹ����ز��ҵ��Ľڵ���, ���򷵻�0
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
*	@brief			����ָ���ڵ������һ�������򲻾����ӽڵ�
*	@param[in]		node: �����ҽڵ�ĸ��ڵ�
*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
*	@return			�ɹ����ز��ҵ��Ľڵ���, ���򷵻�0
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
*	@brief			��ȡָ���ڵ���һ�������򲻾����ֵܽڵ�
*	@param[in]		node: �ڵ���
*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
*	@return			�ɹ����ز��ҵ��Ľڵ���, ���򷵻�0
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
*	@brief			��ȡָ���ڵ���һ�������򲻾����ֵܽڵ�
*	@param[in]		node: �ڵ���
*	@param[in]		nodeN: �����ҵĽڵ�����, ��0��ʾ�����Ҿ����ڵ�
*	@return			�ɹ����ز��ҵ��Ľڵ���, ���򷵻�0
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
*	@brief			����һ���ڵ�������ָ�����ƵĽڵ�
*	@param[in]		node: �����ҽڵ�ĸ��ڵ���
*	@param[in]		nodeN: �����ҵĽڵ�����, ������0
*	@param[in]		nodeS: �ڲ�ʹ��, �ⲿ�������
*	@param[in]		recovR: �ڲ�ʹ��, �ⲿ�������
*	@return			�ɹ����ز��ҵ��Ľڵ���, ���򷵻�0
*	@remark			ѭ�����ô˽ӿ�, ÿ�η���һ���������ƵĽڵ���, ֱ������0������ҽ���
*/
xml_node xml_t::find_all(xml_node node, const char* nodeN, xml_node* nodeS, xml_node* recovR)
{
	if(!nodeN || !nodeS || !recovR) return 0;	//	ֻ�ṩ��������

	TiXmlNode** pNodeS = (TiXmlNode**)nodeS;
	TiXmlNode** pRecov = (TiXmlNode**)recovR;
	TiXmlNode*  iter   = 0;
	TiXmlNode*  pTemp  = 0;

	if(!(*pNodeS))	//	��¼������ʼ��
	{
		iter = *pNodeS = (TiXmlNode*)node;
		if((*pNodeS)->FirstChild()->Type() == TiXmlNode::TEXT) return 0;	//	��ֹ�ӷǸ��ڵ㿪ʼ����
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

	for(;;)	//	ѭ�����Ҿ����ڵ�
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

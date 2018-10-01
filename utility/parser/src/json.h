#ifndef __JSON_H_2015_08_20__
#define __JSON_H_2015_08_20__
#include <utility/parser/src/cJSON.h>
#include <vector>
#include <memory>

typedef enum ITEM_TYPE {
	IT_NONE		= -1,
	IT_FALSE	= 0,
	IT_TRUE		= 1,
	IT_NULL		= 2,
	IT_NUMBER	= 3,
	IT_STRING	= 4,
	IT_ARRAY	= 5,
	IT_OBJECT	= 6
} JSON_ITEM_TYPE;

#pragma pack(push)
#pragma pack(1)

union VALUE {
	char*	str_val;
	struct INT_VALUE
	{
		int	int_val;
		double	double_val;
	} iv;
};

#pragma pack(pop)

typedef void* item_handle;

struct json_t
{
public :
	json_t();
	~json_t();

	item_handle parse(const char* str);
	item_handle parse_from_file(const char* file);
	char*		print(item_handle ih);
	item_handle get_next_item(item_handle ih);
	item_handle get_prev_item(item_handle ih);
	item_handle get_sub_item(item_handle ih);
	item_handle get_item_from_name(item_handle obj, const char* name);
	item_handle get_array_item_from_pos(item_handle arr, int pos);
	ITEM_TYPE	get_item_type(item_handle ih);
	const char* get_item_name(item_handle ih);
	VALUE		get_item_value(item_handle ih, ITEM_TYPE* it);
	int			get_array_size(item_handle ih);
	item_handle create_root();
	item_handle create_null();
	item_handle create_true();
	item_handle create_false();
	item_handle create_bool(bool b);
	item_handle create_number(double num);
	item_handle create_string(const char* str);
	item_handle create_array();
	item_handle create_object();
	item_handle create_int_array(int* arr, int count);
	item_handle create_double_array(double* arr, int count);
	item_handle create_string_array(const char** arr, int count);
	void		add_item_to_array(item_handle arr, item_handle item);
	void		add_item_to_object(item_handle obj, const char* name, item_handle item);
	void		detach_item_from_array(item_handle arr, int pos);
	void		delete_item_from_array(item_handle arr, int pos);
	void		detach_item_from_object(item_handle obj, const char* name);
	void		delete_item_from_object(item_handle obj, const char* name);
	void		json_mem_free(void* mem);

private :
	cJSON* handle_;
};
//////////////////////////////////////////////////////////////////////////

class CJSon
{
public :
	CJSon() : json_(), root_(), ih_() { json_ = std::make_shared<json_t>(); }
	~CJSon() {}
	CJSon(const CJSon& json) { *this = json; }
	CJSon& operator = (const CJSon& json)
	{
		if(this == &json) return *this;
		this->json_ = json.json_;
		this->ih_ = json.ih_;
		return *this;
	}

public :
	bool decode_from_stream(const char* str)
	{
		root_ = ih_ = json_->parse(str);
		if(!ih_) return false;
		return true;
	}

	//	暂不支持
	bool decode_from_file(const char* path) { return false; }

	//	指针需调用 json_mem_free 释放
	char* encode()
	{
		return json_->print(root_);
	}

	ITEM_TYPE get_item_type()
	{
		return json_->get_item_type(ih_);
	}

	const char* get_item_name()
	{
		return json_->get_item_name(ih_);
	}

	CJSon& get_next_item()
	{
		ih_ = json_->get_next_item(ih_);
		return *this;
	}

	CJSon& get_prev_item()
	{
		ih_ = json_->get_prev_item(ih_);
		return *this;
	}

	CJSon& get_sub_item()
	{
		if(ih_) stack_.push_back(ih_);
		ih_ = json_->get_sub_item(ih_);
		return *this;
	}

	CJSon& get_item_from_name(const char* name)
	{
		stack_.push_back(ih_);
		ih_ = json_->get_item_from_name(ih_, name);
		return *this;
	}

	VALUE get_item_value(ITEM_TYPE* it = 0)
	{
		return json_->get_item_value(ih_, it);
	}

	int get_array_size()
	{
		return json_->get_array_size(ih_);
	}

	CJSon& get_array_item_from_pos(int pos)
	{
		stack_.push_back(ih_);
		ih_ = json_->get_array_item_from_pos(ih_, pos);
		return *this;
	}

	CJSon& new_root()
	{
		if (!root_) root_ = ih_ = json_->create_root();
		return *this;
	}

	//	创建对象
	CJSon& new_object(const char* name)
	{
		item_handle ih = 0;
		ih = json_->create_object();
		json_->add_item_to_object(ih_, name, ih);
		stack_.push_back(ih_);
		ih_ = ih;
		return *this;
	}

	//	创建数组
	CJSon& new_array(const char* name)
	{
		item_handle ih = 0;
		ih = json_->create_array();
		json_->add_item_to_object(ih_, name, ih);
		ih_ = ih;
		return *this;
	}

	//	实际分配对象，调用 new_ 系列函数后紧跟调用
	CJSon& attach_object()
	{
		if(!ih_)
		{
			root_ = ih_ = json_->create_object();
			return *this;
		}

		switch(json_->get_item_type(ih_))
		{
		case IT_OBJECT :
			{
			}
			break;
		case IT_ARRAY :
			{
				item_handle pa = ih_;
				stack_.push_back(ih_);
				ih_ = json_->create_object();
				json_->add_item_to_array(pa, ih_);
			}
			break;
		default :
			break;
		}
		return *this;
	}

	CJSon& set_int32(const char* name, int v)
	{
		return set_double(name, (double)v);
	}

	CJSon& set_int64(const char* name, __int64 v)
	{
		return set_double(name, (double)v);
	}

	CJSon& set_double(const char* name, double v)
	{
		item_handle ih = 0;
		ih = json_->create_number((double)v);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	CJSon& set_bool(const char* name, bool v)
	{
		item_handle ih = 0;
		ih = json_->create_bool(v);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	CJSon& set_string(const char* name, const char* v)
	{
		item_handle ih = 0;
		ih = json_->create_string(v);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	CJSon& set_int_group(const char* name, int* group, int count)
	{
		item_handle ih = 0;
		ih = json_->create_int_array(group, count);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	CJSon& set_double_group(const char* name, double* group, int count)
	{
		item_handle ih = 0;
		ih = json_->create_double_array(group, count);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	CJSon& set_string_group(const char* name, const char** group, int count)
	{
		item_handle ih = 0;
		ih = json_->create_string_array(group, count);
		json_->add_item_to_object(ih_, name, ih);
		return *this;
	}

	void delete_item_from_array(int pos)
	{
		if(get_item_type() != IT_ARRAY) return;
		int size = get_array_size();
		if(pos >= size) return;
		json_->delete_item_from_array(ih_, pos);
	}

	void delete_item_from_object(const char* name)
	{
		//if(get_item_type() != IT_OBJECT) return;
		if(!name) return;
		json_->delete_item_from_object(ih_, name);
	}

	//	返回逻辑上的上一级节点
	CJSon& to_upper_level()
	{
		if(stack_.size() == 0) return *this;
		ih_ = stack_.back();
		stack_.pop_back();
		return *this;
	}

	//	返回根节点
	CJSon& get_root()
	{
		ih_ = root_;
		stack_.clear();
		return *this;
	}

	//	内存释放
	void json_mem_free(void* mem)
	{
		json_->json_mem_free(mem);
	}

	bool valid()
	{
		return ih_ != 0;
	}

private :
	std::shared_ptr<json_t> json_;
	item_handle root_;					//	根节点
	item_handle ih_;					//	当前操作句柄
	std::vector<item_handle> stack_;	//	句柄链
};

#endif

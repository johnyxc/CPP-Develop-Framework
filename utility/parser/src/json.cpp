#include "stdafx.h"
#include <utility/parser/src/json.h>
#include <bas/memory.hpp>

json_t::json_t() : handle_()
{
	cJSON_Hooks jh = {};
	jh.malloc_fn = mem_alloc;
	jh.free_fn	 = mem_free;
	cJSON_InitHooks(&jh);
}

json_t::~json_t() { if(handle_) cJSON_Delete(handle_); }

item_handle json_t::parse(const char* str)
{
	if(!str) return 0;
	handle_ = cJSON_Parse(str);
	return (item_handle)handle_;
}

item_handle json_t::parse_from_file(const char* file)
{
	return 0;
}

char* json_t::print(item_handle ih)
{
	if(ih) return cJSON_Print((cJSON*)ih);
	return 0;
}

item_handle json_t::get_next_item(item_handle ih)
{
	if(!ih) return 0;
	cJSON* js = (cJSON*)ih;
	return (item_handle)(js->next);
}

item_handle json_t::get_prev_item(item_handle ih)
{
	if(!ih) return 0;
	cJSON* js = (cJSON*)ih;
	return (item_handle)(js->prev);
}

item_handle json_t::get_sub_item(item_handle ih)
{
	if(!ih) return 0;
	cJSON* js = (cJSON*)ih;
	return (item_handle)(js->child);
}

item_handle json_t::get_item_from_name(item_handle obj, const char* name)
{
	if(!obj || !name) return 0;
	return (item_handle)cJSON_GetObjectItem((cJSON*)obj, name);
}

item_handle json_t::get_array_item_from_pos(item_handle arr, int pos)
{
	if(!arr || pos < 0) return 0;
	return (item_handle)cJSON_GetArrayItem((cJSON*)arr, pos);
}

ITEM_TYPE json_t::get_item_type(item_handle ih)
{
	if(!ih) return IT_NONE;

	cJSON* js = (cJSON*)ih;
	switch(js->type)
	{
	case cJSON_False :
		return IT_FALSE;
	case cJSON_True :
		return IT_TRUE;
	case cJSON_NULL :
		return IT_NULL;
	case cJSON_Number :
		return IT_NUMBER;
	case cJSON_String :
		return IT_STRING;
	case cJSON_Array :
		return IT_ARRAY;
	case cJSON_Object :
		return IT_OBJECT;
	default :
		return IT_NONE;
	}
}

const char* json_t::get_item_name(item_handle ih)
{
	if(!ih) return 0;
	cJSON* js = (cJSON*)ih;
	return js->string;
}

VALUE json_t::get_item_value(item_handle ih, ITEM_TYPE* it)
{
	if(!ih) return VALUE();
	if(it) *it = IT_NONE;

	cJSON* js = (cJSON*)ih;
	switch(js->type)
	{
	case cJSON_True :
	case cJSON_False :
		{
			VALUE v;
			v.iv.int_val = js->valueint;
			if(it) *it = (js->type == cJSON_True) ? IT_TRUE : IT_FALSE;
			return v;
		}
	case cJSON_String :
		{
			VALUE v;
			v.str_val = js->valuestring;
			if(it) *it = IT_STRING;
			return v;
		}
		break;
	case cJSON_Number :
		{
			VALUE v;
			v.iv.int_val = js->valueint;
			v.iv.double_val = js->valuedouble;
			if(it) *it = IT_NUMBER;
			return v;
		}
		break;
	default : break;
	}

	return VALUE();
}

int json_t::get_array_size(item_handle ih)
{
	if(!ih) return 0;
	return cJSON_GetArraySize((cJSON*)ih);
}

item_handle json_t::create_root()
{
	return (item_handle)(handle_ = cJSON_CreateObject());
}

item_handle json_t::create_null()
{
	return (item_handle)cJSON_CreateNull();
}

item_handle json_t::create_true()
{
	return (item_handle)cJSON_CreateTrue();
}

item_handle json_t::create_false()
{
	return (item_handle)cJSON_CreateFalse();
}

item_handle json_t::create_bool(bool b)
{
	return (item_handle)cJSON_CreateBool((int)b);
}

item_handle json_t::create_number(double num)
{
	return (item_handle)cJSON_CreateNumber(num);
}

item_handle json_t::create_string(const char* str)
{
	if(!str) return 0;
	return (item_handle)cJSON_CreateString(str);
}

item_handle json_t::create_array()
{
	return (item_handle)cJSON_CreateArray();
}

item_handle json_t::create_object()
{
	return (item_handle)cJSON_CreateObject();
}

item_handle json_t::create_int_array(int* arr, int count)
{
	return (item_handle)cJSON_CreateIntArray(arr, count);
}

item_handle json_t::create_double_array(double* arr, int count)
{
	return (item_handle)cJSON_CreateDoubleArray(arr, count);
}

item_handle json_t::create_string_array(const char** arr, int count)
{
	return (item_handle)cJSON_CreateStringArray(arr, count);
}

void json_t::add_item_to_array(item_handle arr, item_handle item)
{
	cJSON_AddItemToArray((cJSON*)arr, (cJSON*)item);
}

void json_t::add_item_to_object(item_handle obj, const char* name, item_handle item)
{
	cJSON_AddItemToObject((cJSON*)obj, name, (cJSON*)item);
}

void json_t::detach_item_from_array(item_handle arr, int pos)
{
	cJSON_DetachItemFromArray((cJSON*)arr, pos);
}

void json_t::delete_item_from_array(item_handle arr, int pos)
{
	cJSON_DeleteItemFromArray((cJSON*)arr, pos);
}

void json_t::detach_item_from_object(item_handle obj, const char* name)
{
	cJSON_DetachItemFromObject((cJSON*)obj, name);
}

void json_t::delete_item_from_object(item_handle obj, const char* name)
{
	cJSON_DeleteItemFromObject((cJSON*)obj, name);
}

void json_t::json_mem_free(void* mem)
{
	if(mem) mem_free(mem);
}

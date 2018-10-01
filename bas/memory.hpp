#ifndef __MEMORY_HPP_2015_07_14__
#define __MEMORY_HPP_2015_07_14__

//	��װ�ڴ���䡢���󴴽��ӿ�
#include <stdlib.h>
#include <string.h>
#include <bas/repeat.hpp>
#include <bas/mem_pool.hpp>
#include <new>
#include <assert.h>

static void* mem_alloc(size_t size)
{
	//void* p = bas::detail::mem_pool_manager_t::instance()->alloc(size);
	void* p = malloc(size);
	//assert(p != nullptr);
	return p;
}

static void* mem_zalloc(size_t size)
{
	//void* p = bas::detail::mem_pool_manager_t::instance()->alloc(size);
	void* p = malloc(size);
	//assert(p != nullptr);
	memset(p, 0, size);
	return p;
}

static void mem_free(void* ptr)
{
	//bas::detail::mem_pool_manager_t::instance()->free(ptr);
	//assert(ptr != nullptr);
	free(ptr);
}

static char* mem_strdup(const char* str)
{
// 	int len = strlen(str);
// 	char* p = (char*)mem_alloc(len + 1);
// 	strcpy(p, str);
// 	p[len] = '\0';
// 	return p;
	//assert(str != nullptr);
	return strdup(str);
}

static void* mem_copy(void* dst, void const* src, size_t size)
{
	//assert(dst != nullptr && src != nullptr);
	return memmove(dst, src, size);
}

static void* mem_zero(void* ptr, size_t size)
{
	//assert(ptr != nullptr);
	return memset(ptr, 0, size);
}

//	���󴴽�
template <typename T>
static T* mem_create_object()
{
	void* buf = mem_alloc(sizeof(T));
	return new (buf)T();
}

#define mem_create_decl(i) \
	template <typename T, comma_expand(exp_template_list, i)> \
	T* mem_create_object(comma_expand(exp_formal_list, i)) \
{ \
	void* buf = mem_alloc(sizeof(T)); \
	return new (buf)T(comma_expand(exp_actual_list, i)); \
}

blank_expand(mem_create_decl, 9);

//	�����ͷ�
template <typename T>
static void mem_delete_object(T* o)
{
	//assert(o != nullptr);
	o->~T();
	mem_free((void*)o);
}

#endif

#include "msg_encpt.h"

#include "openssl/rc4.h"

#ifdef _M_IX86  
#pragma comment(lib, "openssl/ssleay32.lib") 
#pragma comment(lib, "openssl/libeay32.lib")  
#endif 

#ifdef _M_X64  
#pragma comment(lib, "openssl/libeay32_64.lib")  
#pragma comment(lib, "openssl/ssleay32_64.lib") 
#endif 

static string jf_msg_key = "*#@yunjifei@#*";

shared_ptr<char> jf_msg_codec(const char *ptrData, size_t dataLen)
{
	if (!dataLen) return shared_ptr<char>();

	RC4_KEY key;
	RC4_set_key(&key, (int)jf_msg_key.length(), (const unsigned char *)jf_msg_key.c_str()); //设置密钥  

	shared_ptr<char> buf(new char[dataLen], [](char *p){ delete[]p; });

	RC4(&key, dataLen, (const unsigned char *)ptrData, (unsigned char*)buf.get());	//加密明文  

	return buf;
}


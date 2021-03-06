#include "msg_encpt.h"
#include "openssl/rc4.h"

#pragma comment(lib, "Crypt32.lib")

#ifdef _M_IX86
#pragma comment(lib, "openssl/libssl.lib") 
#pragma comment(lib, "openssl/libcrypto.lib") 
#endif 

#ifdef _M_X64
#pragma comment(lib, "openssl/libssl64.lib") 
#pragma comment(lib, "openssl/libcrypto64.lib")  
#endif 

static string jf_msg_key = "*#@yunjifei@#*";

shared_ptr<char> jf_msg_codec(const char *ptrData, size_t dataLen)
{
	if (!dataLen) return shared_ptr<char>();

	RC4_KEY key;
	RC4_set_key(&key, (int)jf_msg_key.length(), (const unsigned char *)jf_msg_key.c_str()); //������Կ  

	shared_ptr<char> buf(new char[dataLen], [](char *p){ delete[]p; });

	RC4(&key, dataLen, (const unsigned char *)ptrData, (unsigned char*)buf.get());	//��������  

	return buf;
}


#ifndef __RC4__H__
#define __RC4__H__

//key 秘钥
//data 加密或者解密的数据
//data_len 输出加密后数据长度
//data_len 输入解密数据长度
void rc4Decrypt(const char *key, char *data, int *data_len);

#endif

#ifndef __MSG_ENCPT_HPP_2017_12_19__
#define __MSG_ENCPT_HPP_2017_12_19__

#include <string>
#include <memory>
using namespace std;

shared_ptr<char> jf_msg_codec(const char *ptrData, size_t dataLen);

#endif

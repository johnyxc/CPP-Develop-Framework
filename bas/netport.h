#ifndef __NETPORT_H_2017_04_06__
#define __NETPORT_H_2017_04_06__
/*
*	网络传输端口对象定义
*
*	职责：
*	1、不关心 socket 来源，只关心其是否能正常工作
*	2、使用标准头接口，上层自行实现所有接口函数
*	3、发送上层传来的数据，接收已绑定的 socket 的数据，回调上层
*	4、基本收取数据模式为：先收头部，根据头部中标示的消息体长度再收消息体，如此往复
*	单次最多收取 2048 字节数据，若消息体超过 2048 字节，则分多次收取
*/
#include <functional>
#include <memory>

namespace bas {
	namespace detail {
		struct socket_t;
		struct strand_t;
	}
};

using namespace bas;
using namespace bas::detail;
struct standard_header;

//	网络传输端口对象
struct netport_t :
	std::enable_shared_from_this<netport_t>
{
	enum TCP_PARAM { TP_MAX_RECV_SIZE = 2048 };
	using error_callback	= std::function<void (int)>;							//	签名：错误码
	using recv_callback		= std::function<void (const char*, const char*, int)>;	//	签名 : 命令头 命令体数据 命令体长度
	using send_callback		= std::function<void (int)>;							//	签名：发送长度

public :
	netport_t(standard_header* hdr);
	~netport_t();

public :
	void set_strand(std::shared_ptr<strand_t> strand);
	void clear();
	void bind_socket(std::shared_ptr<socket_t> sock);
	void set_error_callback(error_callback cb);
	void set_recv_callback(recv_callback cb);
	void set_send_callback(send_callback cb);
	void send_message(const char* buf, int len);
	void send_message(const std::string& buf, int len);

private :
	struct netport_impl_t;
	std::shared_ptr<netport_impl_t> impl_;
};

#endif

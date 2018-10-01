#ifndef __NETPORT_H_2017_04_06__
#define __NETPORT_H_2017_04_06__
/*
*	���紫��˿ڶ�����
*
*	ְ��
*	1�������� socket ��Դ��ֻ�������Ƿ�����������
*	2��ʹ�ñ�׼ͷ�ӿڣ��ϲ�����ʵ�����нӿں���
*	3�������ϲ㴫�������ݣ������Ѱ󶨵� socket �����ݣ��ص��ϲ�
*	4��������ȡ����ģʽΪ������ͷ��������ͷ���б�ʾ����Ϣ�峤��������Ϣ�壬�������
*	���������ȡ 2048 �ֽ����ݣ�����Ϣ�峬�� 2048 �ֽڣ���ֶ����ȡ
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

//	���紫��˿ڶ���
struct netport_t :
	std::enable_shared_from_this<netport_t>
{
	enum TCP_PARAM { TP_MAX_RECV_SIZE = 2048 };
	using error_callback	= std::function<void (int)>;							//	ǩ����������
	using recv_callback		= std::function<void (const char*, const char*, int)>;	//	ǩ�� : ����ͷ ���������� �����峤��
	using send_callback		= std::function<void (int)>;							//	ǩ�������ͳ���

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

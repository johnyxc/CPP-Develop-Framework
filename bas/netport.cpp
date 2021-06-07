#include <bas/netport.h>
#include <bas/strand.hpp>
#include <bas/memory.hpp>
#include <bas/socket.hpp>
#include <bas/thread_pool.hpp>
#include <protocol/protocol.hpp>
#include <utility/Log/Log.h>
#include <utility/decoder/msg_encpt.h>
#include <list>
using namespace jf_log;
using namespace std::placeholders;

struct netport_t::netport_impl_t :
	std::enable_shared_from_this<netport_t::netport_impl_t>
{
	struct cmd_info
	{
		char*	cmd_hdr;
		char*	cmd_body;
		int		body_len;
	};

public :
	netport_impl_t(standard_header* hdr) :
		std_hdr_(hdr),
		cur_send_(),
		recv_cb_(),
		send_cb_(),
		err_cb_(),
		has_recved_()
	{
		mem_zero((void*)&ci_, sizeof(cmd_info));
	}
	
	~netport_impl_t()
	{
	}

public :
	/*
	*	post_call��ͨ��ͬһ�� strand_t ���� post_call Ͷ��ִ�еĺ�������
	*	һ���Ǵ��л�ִ�еģ��������������Դ�������ִ��ʱ�䲻��ȷ��������
	*	Ͷ�ݵĺ�������һ����ִ��
	*	post_call ���Ŀ���Ǳ���Ƶ��ʹ��������ʱ���������ɵ��������⣬
	*	�Ե�������˵��ʱ��ò�����������Ȩ����Ӱ���ϲ�Ч��
	*	post_call ��������ȫ����ʹ��������������ʱ����Ҫ������post_call
	*	��Ҫ���в���������Ȩ�����Ҫ�ر�ע��
	*/
	void clear()
	{
		default_thread_pool()->post_call(std::bind(&netport_impl_t::i_do_clear,
			shared_from_this()), strand_);
	}

	void bind_socket(std::shared_ptr<socket_t> sock)
	{
		default_thread_pool()->post_call(std::bind(&netport_impl_t::i_bind_socket_strand,
			shared_from_this(), sock), strand_);
	}

	void i_bind_socket_strand(std::shared_ptr<socket_t> sock)
	{
		sock_ = sock;
		sock_->bind_send_callback(std::bind(&netport_impl_t::i_on_send, shared_from_this(), _1, _2));
		sock_->bind_recv_callback(std::bind(&netport_impl_t::i_on_recv, shared_from_this(), _1, _2));
		sock_->bind_error_callback(std::bind(&netport_impl_t::i_on_error, shared_from_this(), _1));

		ci_.cmd_hdr = (char*)mem_zalloc(std_hdr_->get_header_len());
		flag_ = 1;
		sock_->async_recv(ci_.cmd_hdr, std_hdr_->get_header_len());
	}

	/*
	*	����ͬһ�� strand_t ������࣬
	*	post_call ����ͬһ�����л�������ִ��
	*/
	void set_strand(std::shared_ptr<strand_t> strand)
	{
		strand_ = strand;
	}

	void set_error_callback(netport_t::error_callback cb)
	{
		err_cb_ = cb;
	}

	void set_recv_callback(netport_t::recv_callback cb)
	{
		recv_cb_ = cb;
	}

	void set_send_callback(send_callback cb)
	{
		send_cb_ = cb;
	}

	void send_message(const char* buf, int len)
	{
		if (!buf || !len) return;

		cmd_info* ci = (cmd_info*)mem_zalloc(sizeof(cmd_info));
		if (!ci) return;

		ci->cmd_body = (char*)mem_alloc(len);
		ci->body_len = len;
		if (!ci->cmd_body)
		{
			mem_free((void*)ci);
			return;
		}

		//	����
		int hdr_len = std_hdr_->get_header_len();

		auto ept_buf = jf_msg_codec(buf + hdr_len, len - hdr_len);
		if (ept_buf.get())
		{
			mem_copy((void*)ci->cmd_body, (void*)buf, hdr_len);
			mem_copy((void*)(ci->cmd_body + hdr_len), (void*)ept_buf.get(), (len - hdr_len));
			default_thread_pool()->post_call(
				std::bind(&netport_impl_t::i_send_message_strand, shared_from_this(), ci),
				strand_);
		}
		else
		{
			mem_copy((void*)ci->cmd_body, (void*)buf, hdr_len);
			default_thread_pool()->post_call(
				std::bind(&netport_impl_t::i_send_message_strand, shared_from_this(), ci),
				strand_);
		}
	}

	void send_message(const std::string& buf, int len)
	{
		send_message(buf.c_str(), len);
	}

private :
	void i_send_message_strand(cmd_info* ci)
	{
		if (!sock_->valid())
		{
			if (ci)
			{
				if (ci->cmd_hdr) mem_free((void*)ci->cmd_hdr);
				if (ci->cmd_body) mem_free((void*)ci->cmd_body);
				mem_free((void*)ci);
			}
			return;
		}

		if (cur_send_)
		{
			cmd_info_list_.push_back(ci);
		}
		else
		{
			cur_send_ = ci;
			sock_->async_send(ci->cmd_body, ci->body_len);
		}
	}

	void i_on_recv(int bt, int err)
	{
		default_thread_pool()->post_call(
			std::bind(&netport_impl_t::i_on_recv_strand, shared_from_this(), bt, err), strand_);
	}

	void i_on_recv_strand(int bt, int err)
	{
		if (err || !sock_->valid()) return;

		switch (flag_)
		{
		case 0 :
			{	//	���հ�ͷ
				i_process_command(ci_.cmd_body, ci_.body_len);
				if (ci_.cmd_body) mem_free((void*)ci_.cmd_body);
				ci_.body_len = 0;
				ci_.cmd_body = 0;

				flag_ = 1;
				mem_zero((void*)ci_.cmd_hdr, std_hdr_->get_header_len());
				sock_->async_recv(ci_.cmd_hdr, std_hdr_->get_header_len());
			}
			break;
		case 1 :
			{	//	���հ���
				standard_header* sh = std_hdr_->clone(ci_.cmd_hdr, std_hdr_->get_header_len());
				if (!sh)
				{
					i_on_error(7);
					break;
				}

				int body_len = sh->get_body_len();
				sh->release();

				if (body_len > 0)
				{
					ci_.cmd_body = (char*)mem_alloc(body_len);
					ci_.body_len = body_len;

					int recv_size = ci_.body_len;
					if (recv_size > TP_MAX_RECV_SIZE) {
						flag_ = 2;
						recv_size = TP_MAX_RECV_SIZE;
					} else {
						flag_ = 0;
					}

					sock_->async_recv(ci_.cmd_body, recv_size);
				}
				else
				{
					flag_ = 0;
					i_on_recv_strand(0, 0);
				}
			}
			break;
		case 2:
			{	//	���������������
				has_recved_ += bt;
				if (has_recved_ == ci_.body_len)
				{
					has_recved_ = 0;
					flag_ = 0;
					i_on_recv_strand(0, 0);
				}
				else
				{
					int recv_size = ci_.body_len - has_recved_;
					if (recv_size > TP_MAX_RECV_SIZE) recv_size = TP_MAX_RECV_SIZE;
					sock_->async_recv(ci_.cmd_body + has_recved_, recv_size);
				}
			}
			break;
		default :
			break;
		}
	}

	void i_on_send(int bt, int err)
	{
		default_thread_pool()->post_call(
			std::bind(&netport_impl_t::i_on_send_strand, shared_from_this(), bt, err),
			strand_);
	}

	void i_on_send_strand(int bt, int err)
	{
		if (send_cb_) send_cb_(bt);

		if (cur_send_)
		{
			if (cur_send_->cmd_hdr) mem_free((void*)cur_send_->cmd_hdr);
			cur_send_->cmd_hdr = 0;

			if (cur_send_->cmd_body) mem_free((void*)cur_send_->cmd_body);
			cur_send_->cmd_body = 0;

			mem_free((void*)cur_send_);
			cur_send_ = 0;
		}

		if (cmd_info_list_.size())
		{
			cmd_info* ci = cmd_info_list_.front();
			cmd_info_list_.pop_front();
			i_send_message_strand(ci);
		}
	}

	void i_on_error(int err)
	{
		default_thread_pool()->post_call(
			std::bind(&netport_impl_t::i_on_error_strand, shared_from_this(), err),
			strand_);
	}

	/*
	*	��֪ͨ�ϲ� socket ����
	*	������ netport �ڲ���Դ
	*/
	void i_on_error_strand(int err)
	{
		if (err_cb_) err_cb_(err);
		i_do_clear();
	}

	void i_process_command(const char* body, int len)
	{
		if (!recv_cb_) return;

		//	����
		auto dpt_buf = jf_msg_codec(ci_.cmd_body, ci_.body_len);
		if (dpt_buf.get())
		{
			recv_cb_(ci_.cmd_hdr, dpt_buf.get(), ci_.body_len);
		}
		else
		{
			recv_cb_(ci_.cmd_hdr, 0, 0);
		}
	}

	void i_do_clear()
	{
		if (recv_cb_) recv_cb_ = {};
		if (send_cb_) send_cb_ = {};
		if (err_cb_)  err_cb_  = {};
		//	���Ӽ�⣬�Ӷ��ڳ���ֹͣʱ����dump�ļ�����״̬
		if (sock_->valid())
		{
			LOG(LT_DEBUG) << "Netport Close Socket : " << reinterpret_cast<int>(sock_.get()) << LOG_END;
			sock_->close();
		}

		for (auto iter : cmd_info_list_)
		{
			if (iter->cmd_hdr)	mem_free((void*)iter->cmd_hdr);
			if (iter->cmd_body) mem_free((void*)iter->cmd_body);
			mem_free((void*)iter);
		}
		cmd_info_list_.clear();

		if (cur_send_)
		{
			if(cur_send_->cmd_hdr)	mem_free((void*)cur_send_->cmd_hdr);
			if(cur_send_->cmd_body)	mem_free((void*)cur_send_->cmd_body);
			mem_free((void*)cur_send_);
			cur_send_ = 0;
		}

		if (ci_.cmd_hdr)
		{
			mem_free((void*)ci_.cmd_hdr);
			ci_.cmd_hdr = 0;
		}

		if (ci_.cmd_body)
		{
			mem_free((void*)ci_.cmd_body);
			ci_.cmd_body = 0;
		}
	}

private :
	int										flag_;				//	�ֶ���ȡ�׶α�ʾ
	int										has_recved_;		//	����ȡ����
	cmd_info								ci_;				//	ÿ����ȡʹ�õĽṹ
	std::shared_ptr<socket_t>				sock_;				//	socket_tģ��
	netport_t::error_callback				err_cb_;			//	����ص�
	netport_t::recv_callback				recv_cb_;			//	���ջص�
	netport_t::send_callback				send_cb_;			//	���ͻص�
	standard_header*						std_hdr_;			//	��׼ͷ����ṹ
	cmd_info*								cur_send_;			//	��ǰ���ڷ��͵�����
	std::shared_ptr<strand_t>				strand_;			//	���л���
	std::list<cmd_info*>					cmd_info_list_;		//	���Ͷ���
};
//////////////////////////////////////////////////////////////////////////

netport_t::netport_t(standard_header* hdr)
{
	impl_ = std::make_shared<netport_impl_t>(hdr);
}

netport_t::~netport_t() {}

void netport_t::bind_socket(std::shared_ptr<socket_t> sock)
{
	impl_->bind_socket(sock);
}

void netport_t::set_strand(std::shared_ptr<strand_t> strand)
{
	impl_->set_strand(strand);
}

void netport_t::clear()
{
	impl_->clear();
}

void netport_t::set_error_callback(error_callback cb)
{
	impl_->set_error_callback(cb);
}

void netport_t::set_recv_callback(recv_callback cb)
{
	impl_->set_recv_callback(cb);
}

void netport_t::set_send_callback(send_callback cb)
{
	impl_->set_send_callback(cb);
}

void netport_t::send_message(const char* buf, int len)
{
	impl_->send_message(buf, len);
}

void netport_t::send_message(const std::string& buf, int len)
{
	impl_->send_message(buf, len);
}

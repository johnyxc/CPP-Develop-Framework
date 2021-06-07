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
	*	post_call：通过同一个 strand_t 对象 post_call 投递执行的函数对象，
	*	一定是串行化执行的，不会出现争抢资源的情况，执行时间不能确定，但先
	*	投递的函数对象一定先执行
	*	post_call 设计目的是避免频繁使用锁，有时因疏忽而造成的死锁问题，
	*	对调用者来说长时间得不到锁的所有权，会影响上层效率
	*	post_call 并不是完全放弃使用锁，该用锁的时候还是要用锁，post_call
	*	需要持有参数的所有权，这点要特别注意
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
	*	持有同一个 strand_t 对象的类，
	*	post_call 都在同一个串行化环境中执行
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

		//	加密
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
			{	//	接收包头
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
			{	//	接收包体
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
			{	//	命令超长，持续接收
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
	*	先通知上层 socket 出错
	*	再清理 netport 内部资源
	*/
	void i_on_error_strand(int err)
	{
		if (err_cb_) err_cb_(err);
		i_do_clear();
	}

	void i_process_command(const char* body, int len)
	{
		if (!recv_cb_) return;

		//	解密
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
		//	不加检测，从而在程序停止时产生dump文件分析状态
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
	int										flag_;				//	分段收取阶段标示
	int										has_recved_;		//	已收取长度
	cmd_info								ci_;				//	每次收取使用的结构
	std::shared_ptr<socket_t>				sock_;				//	socket_t模块
	netport_t::error_callback				err_cb_;			//	错误回调
	netport_t::recv_callback				recv_cb_;			//	接收回调
	netport_t::send_callback				send_cb_;			//	发送回调
	standard_header*						std_hdr_;			//	标准头处理结构
	cmd_info*								cur_send_;			//	当前正在发送的数据
	std::shared_ptr<strand_t>				strand_;			//	串行化类
	std::list<cmd_info*>					cmd_info_list_;		//	发送队列
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

#ifndef __SOCKET_HPP_2016_09_19__
#define __SOCKET_HPP_2016_09_19__
#include <asio/asio.hpp>
#include <bas/thread_pool.hpp>
#include <bas/error_def.hpp>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#define SOCKET_FD SOCKET
#define NET_EAGAIN WSAEWOULDBLOCK
#define NET_ERROR WSAGetLastError()
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#define SOCKET_FD int
#define NET_EAGAIN EAGAIN
#define NET_ERROR errno
#endif

SET_MODULE_ERR_BAS(mod_sock, 0)
BEGIN_ERROR_CODE(SOCK)
DEFINE_ERROR_CODE(SOCK_HANDEL_ERR, mod_sock, 1);
DEFINE_ERROR_CODE(RESOLVE_ERR, mod_sock, 2);
DEFINE_ERROR_CODE(CONNECT_ERR, mod_sock, 3);
DEFINE_ERROR_CODE(READ_ERR, mod_sock, 4);
DEFINE_ERROR_CODE(WRITE_ERR, mod_sock, 5);
DEFINE_ERROR_CODE(PARAM_ERR, mod_sock, 6);
END_ERROR_CODE()

namespace bas
{
	namespace detail
	{
#define MAX_BLOCK_SIZE	(1 << 14)

		//	基础套接字对象，提供一些公用函数
		struct socket_base_t
		{
		public :
			socket_base_t() {}
			virtual ~socket_base_t() {}

		public :
			static void set_non_block(asio::ip::tcp::socket* sock)
			{
			}

			static void set_no_delay(SOCKET_FD sock)
			{
				int flag = 1;
				::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
			}

			static void set_reuse(asio::ip::tcp::socket* sock)
			{
			}
		};

		//	域名解析对象
		template <typename T = asio::ip::tcp>
		struct resolver_t :
			std::enable_shared_from_this<resolver_t<T>>
		{
			typedef typename T::resolver			real_type;
			typedef typename real_type::iterator	real_iter;
			using resolve_callback = std::function<void (real_iter&)>;

		public:
			resolver_t() : resolver_(default_thread_pool()->get_service()) {}
			~resolver_t() {}

		public:
			bool async_resolve(const char* url, unsigned short port, resolve_callback cb)
			{
				char port_ptr[10] = {};
				sprintf(port_ptr, "%d", port);
				real_type::query query(url, port_ptr);

				auto shared_this = shared_from_this();
				resolver_.async_resolve(query,
					[shared_this, this, cb](const std::error_code& err, real_iter ei) {
					cb(ei);
				});

				return true;
			}

		private:
			real_type resolver_;
		};

		//	UDP套接字对象
		struct udp_socket_t :
			std::enable_shared_from_this<udp_socket_t>
		{
			using send_cb = std::function<void (int, std::size_t)>;
			using recv_cb = std::function<void (int, std::size_t)>;

		public :
			udp_socket_t(asio::ip::udp::endpoint ep)
			{
				ep_ = ep;
				udp_sock_ = std::make_shared<asio::ip::udp::socket>(default_thread_pool()->get_service());
				udp_sock_->open(asio::ip::udp::v4());
			}
			~udp_socket_t() {}

		public :
			void send(const char* buf, int len, unsigned short port, send_cb cb)
			{
				if (!buf || !len) return;

				asio::ip::udp::endpoint tmp_ep(ep_.address(), port);
				auto shared_this = shared_from_this();
				udp_sock_->async_send_to(asio::buffer(buf, len), tmp_ep, [cb, shared_this](const std::error_code& err, std::size_t bt) {
					if (cb) cb(err.value(), bt);
				});
			}

			void recv_from(char* buf, int max_len, recv_cb cb)
			{
				if (!buf || !max_len) return;

				auto shared_this = shared_from_this();
				asio::ip::udp::endpoint ep;
				udp_sock_->async_receive_from(asio::buffer(buf, max_len), ep, [cb, shared_this](const std::error_code& err, std::size_t bt) {
					if (cb) cb(err.value(), bt);
				});
			}

			void get_peer_info(std::string& addr, unsigned short& port)
			{
				asio::error_code ec;
				addr = ep_.address().to_string(ec);
				port = ep_.port();
			}

		private :
			asio::ip::udp::endpoint					ep_;
			std::shared_ptr<asio::ip::udp::socket>	udp_sock_;
		};

		struct udp_socket_initor_t :
			std::enable_shared_from_this<udp_socket_initor_t>
		{
			using init_res_cb = std::function<void (std::shared_ptr<udp_socket_t>)>;
			using cli_socket_cb = std::function<void (std::shared_ptr<udp_socket_t>, int)>;

		public:
			udp_socket_initor_t() {}
			~udp_socket_initor_t() {}

		public:
			bool init_as_server(unsigned short port, char* buf, int buf_len, cli_socket_cb cb)
			{
				svr_sock_ = std::make_shared<asio::ip::udp::socket>(
					default_thread_pool()->get_service(),
					asio::ip::udp::endpoint(asio::ip::udp::v4(), port)
					);

				svr_sock_->set_option(asio::socket_base::reuse_address(true));
				svr_sock_->async_receive_from(asio::buffer(buf, buf_len), rmt_ep_,
					std::bind(&udp_socket_initor_t::i_on_client_message, shared_from_this(),
					std::placeholders::_1, std::placeholders::_2, svr_sock_, cb, buf, buf_len));

				return true;
			}

			bool init_as_client(const char* ip, unsigned short port, init_res_cb cb)
			{
				resolve_ = std::make_shared<resolver_t<asio::ip::udp>>();

				auto shared_this = shared_from_this();
				resolve_->async_resolve(ip, port,
					[shared_this, this, cb](asio::ip::udp::resolver::iterator& iter) {
					asio::ip::udp::resolver::iterator end;
					if (iter != end)
					{
						std::shared_ptr<udp_socket_t> work = std::make_shared<udp_socket_t>(iter->endpoint());
						cb(work);
					}
				});

				return true;
			}

			void uninit()
			{
				if (svr_sock_) svr_sock_->close();
			}

		private:
			void i_on_client_message(asio::error_code err, size_t bt,
				std::shared_ptr<asio::ip::udp::socket> sock, cli_socket_cb cb, char* buf, int buf_len)
			{
				if (!err)
				{
					std::shared_ptr<udp_socket_t> work = std::make_shared<udp_socket_t>(rmt_ep_);
					if (cb) cb(work, bt);
				}

				sock->async_receive_from(asio::buffer(buf, buf_len), rmt_ep_,
					std::bind(&udp_socket_initor_t::i_on_client_message, shared_from_this(),
					std::placeholders::_1, std::placeholders::_2, sock, cb, buf, buf_len));
			}

		private:
			std::shared_ptr<resolver_t<asio::ip::udp>>	resolve_;
			std::shared_ptr<asio::ip::udp::socket>		svr_sock_;
			asio::ip::udp::endpoint						rmt_ep_;
		};

		//	TCP套接字对象
		struct socket_t :
			std::enable_shared_from_this<socket_t>
		{
			using recv_callback		= std::function<void(int, int)>;	//	签名：接收长度、错误码
			using send_callback		= std::function<void(int, int)>;	//	签名：发送长度、错误码
			using error_callback	= std::function<void(int)>;			//	签名：错误码

		public :
			socket_t()
			{
				tcp_sock_ = std::make_shared<asio::ip::tcp::socket>(
					default_thread_pool()->get_service()
					);
				is_open_ = true;
			}
			socket_t(std::shared_ptr<asio::ip::tcp::socket> sock) :
				tcp_sock_(sock)
			{
				is_open_ = true;
			}
			~socket_t() {}

		public :
			void bind_recv_callback(recv_callback cb)
			{
				recv_cb_ = cb;
			}

			void bind_send_callback(send_callback cb)
			{
				send_cb_ = cb;
			}

			void bind_error_callback(error_callback cb)
			{
				err_cb_ = cb;
			}

			void get_peer_info(std::string& addr, unsigned short& port)
			{
				asio::error_code ec;
				addr = tcp_sock_->remote_endpoint().address().to_string(ec);
				port = tcp_sock_->remote_endpoint().port();
			}

			void set_no_delay()
			{
				if (!tcp_sock_) return;
				asio::ip::tcp::no_delay option(true);
				tcp_sock_->set_option(option);
			}

			void set_buffer_size(int size)
			{
				if (!tcp_sock_) return;

				{
					asio::socket_base::send_buffer_size option(size);
					tcp_sock_->set_option(option);
				}

				{
					asio::socket_base::receive_buffer_size option(size);
					tcp_sock_->set_option(option);
				}
			}

			void set_keepalive()
			{
				asio::socket_base::keep_alive option(true);
				tcp_sock_->set_option(option);
			}

			//	异步接收（指定长度）
			bool async_recv(char* buf, int len)
			{
				if (!buf || !len || !is_open_) return false;

				auto shared_this = shared_from_this();
				asio::async_read(*tcp_sock_.get(), asio::buffer(buf, len),
					[shared_this, this] (const std::error_code& err, std::size_t bt) {
						if (err) {
							error_occur(err.value());
						} else {
							if (recv_cb_) recv_cb_(bt, 0);
						}
					}
				);

				return true;
			}

			//	异步接收（缓冲区有数据立即返回）
			bool async_recv_some(char* buf, int len, recv_callback cb)
			{
				if (!buf || !len || !is_open_) return false;

				auto shared_this = shared_from_this();
				tcp_sock_->async_read_some(asio::buffer(buf, len),
					[shared_this, this, cb](const std::error_code& err, std::size_t bt) {
						if (err) {
							error_occur(err.value());
						} else {
							if (cb) cb(bt, 0);
						}
					}
				);

				return true;
			}

			//	异步发送
			bool async_send(const char* buf, int len)
			{
				if (!buf || !len || !is_open_) return false;

				auto shared_this = shared_from_this();
				tcp_sock_->async_send(asio::buffer(buf, len),
					[shared_this, this](const std::error_code& err, std::size_t bt) {
					if (err) {
						error_occur(err.value());
					} else {
						if (send_cb_) send_cb_(bt, 0);
					}
				});

				return true;
			}

			bool valid()
			{
				return is_open_;
			}

			void close()
			{
				bool tmp_is_open = false;

				{
					std::lock_guard<std::recursive_mutex> lock(mutex_);
					tmp_is_open = is_open_;
					is_open_ = false;
					if (recv_cb_) recv_cb_ = {};
					if (send_cb_) send_cb_ = {};
					if (err_cb_)  err_cb_  = {};
				}

				if (tmp_is_open)
				{
					asio::error_code ec;
					tcp_sock_->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
					tcp_sock_->close(ec);
				}
			}

			asio::ip::tcp::socket*
				get_socket()
			{
				return tcp_sock_.get();
			}

		private :
			void error_occur(int err)
			{
				//	直接采用加锁的方式，防止多线程同时调用 err_cb_
				//	此函数并不会释放 socket，而是由上层调用 close 接口释放
				//	当 socket 错误时，不要尝试重新使用此对象，而是应该新建一个 socket 对象
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				is_open_ = false;
				if (err_cb_) err_cb_(err);
				err_cb_ = {};
			}

		private :
			std::shared_ptr<asio::ip::tcp::socket>	tcp_sock_;
			std::recursive_mutex					mutex_;
			recv_callback							recv_cb_;
			send_callback							send_cb_;
			error_callback							err_cb_;
			bool									is_open_ = false;
		};

		//	连接对象
		struct connector_t :
			std::enable_shared_from_this<connector_t>
		{
			using connect_callback = std::function<void (std::shared_ptr<socket_t>, int)>;

		public :
			connector_t() { resolve_ = std::make_shared<resolver_t<>>(); }
			~connector_t() {}

		public :
			bool async_connect(const char* ip, unsigned short port, connect_callback cb, unsigned int timeout)
			{
				if(!ip || port == 0) return false;
				return i_resolve(ip, port, cb, timeout);
			}

		private :
			bool i_resolve(const char* ip, unsigned short port, connect_callback cb, unsigned int timeout)
			{
				auto shared_this = shared_from_this();
				resolve_->async_resolve(ip, port,
					[shared_this, this, cb, timeout] (asio::ip::tcp::resolver::iterator& iter) {
					asio::ip::tcp::resolver::iterator end;
					if (iter == end) {
						cb(std::shared_ptr<socket_t>(), ERROR_CODE(SOCK, RESOLVE_ERR));
						return;
					}
					i_connect(iter->endpoint(), cb, timeout);
				});
				return true;
			}

			bool i_connect(asio::ip::tcp::endpoint& ep, connect_callback cb, unsigned int timeout)
			{
				auto shared_this = shared_from_this();
				std::shared_ptr<asio::ip::tcp::socket> sock =
					std::make_shared<asio::ip::tcp::socket>(default_thread_pool()->get_service());
				sock->async_connect(ep,
						[shared_this, sock, cb] (const std::error_code& err) {
						if(err) {
							sock->close();
							cb(std::shared_ptr<socket_t>(), ERROR_CODE(SOCK, CONNECT_ERR));
						} else {
							std::shared_ptr<socket_t> shared_sock =
								std::make_shared<socket_t>(sock);

							shared_sock->set_no_delay();
							shared_sock->set_buffer_size(10 * 1024);
							shared_sock->set_keepalive();

							cb(shared_sock, 0);
						}
					}
				);
				return true;
			}

		private :
			std::shared_ptr<resolver_t<>> resolve_;
		};

		//	监听对象
		struct acceptor_t :
			std::enable_shared_from_this<acceptor_t>
		{
			using accept_callback = std::function<void (std::shared_ptr<socket_t>, int)>;

		public :
			acceptor_t() {}
			~acceptor_t() {}

		public :
			void bind_accept_callback(accept_callback cb)
			{
				acpt_cb_ = cb;
			}

			bool async_accept(unsigned short port, int backlog = 1024)
			{
				if (port == 0) return false;

				quit_ = false;
				try
				{
					acceptor_ = std::make_shared<asio::ip::tcp::acceptor>(
						default_thread_pool()->get_service(),
						asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)
						);
					asio::socket_base::reuse_address option(true);
					acceptor_->set_option(option);
				} catch (...) {
					return false;
				}
				i_start_accept();

				return true;
			}

			void stop()
			{
				quit_	 = true;
				acpt_cb_ = {};
				acceptor_->close();
			}

		private :
			void i_start_accept()
			{
				std::shared_ptr<socket_t> sock = std::make_shared<socket_t>();
				auto shared_this = shared_from_this();
				acceptor_->async_accept(*(sock->get_socket()),
					[shared_this, this, sock] (const std::error_code &err) {
					if (!err) {
						sock->set_no_delay();
						sock->set_buffer_size(10 * 1024);
						sock->set_keepalive();
						acpt_cb_(sock, 0);
					}
					
					if (!quit_) i_start_accept();
				});
			}

		private :
			std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
			bool quit_;
			accept_callback acpt_cb_;
		};
	}
}

#endif

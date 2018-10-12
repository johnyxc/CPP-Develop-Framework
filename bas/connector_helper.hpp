#ifndef __CONNECTOR_HELPER_HPP_2016_12_15__
#define __CONNECTOR_HELPER_HPP_2016_12_15__
#include <bas/socket.hpp>
#include <bas/netport.h>
#include <bas/timer.hpp>
#include <protocol/protocol.hpp>
#include <utility/Log/Log.h>
#include <memory>
#include <mutex>
using namespace jf_log;

namespace bas
{
	namespace detail
	{
		struct connector_helper_t :
			std::enable_shared_from_this<connector_helper_t>
		{
			enum NETWORK_PARAM { NP_TIMEOUT = 5000, NP_RETRY_INTERVAL = 15000, NP_HEARTBEAT = 20000 };
			using disconnect_callback	= std::function<void()>;
			using connect_callback		= std::function<void(int)>;
			using send_callback			= std::function<void(int)>;
			using recv_callback			= std::function<void(const char*, const char*, int)>;

		public:
			connector_helper_t(standard_header* hdr) : std_hdr_(hdr), strand_(), auto_reconnect_(true), stopped_(false)
			{
				connector_ = std::make_shared<connector_t>();
			}
			~connector_helper_t() {}

		public:
			void set_strand(std::shared_ptr<strand_t> strand)
			{
				strand_ = strand;
			}

			void start(const char* ip, unsigned short port)
			{
				ip_		= ip;
				port_	= port;

				auto shared_this = shared_from_this();
				connector_->async_connect(ip_.c_str(), port_,
					[shared_this, this](std::shared_ptr<socket_t>sock, int err) {

					std::lock_guard<std::recursive_mutex> lock(mutex_);
					if (stopped_)
					{
						if (!err) sock->close();
						return;
					}

					if (err)
					{
						if (conn_cb_) conn_cb_(1);
						if (auto_reconnect_) i_reconnect();
					}
					else
					{
						netport_ = std::make_shared<netport_t>(std_hdr_);
						auto shared_this = shared_from_this();

						netport_->set_strand(strand_);
						netport_->set_recv_callback([shared_this, this](const char* hdr, const char* body, int len) {
							if (recv_cb_) recv_cb_(hdr, body, len);
						});
						netport_->set_send_callback([shared_this, this](int bt) {
							if (send_cb_) send_cb_(bt);
						});
						netport_->set_error_callback([shared_this, this](int err) {
							LOG(LT_DEBUG) << "Connector Helper Error : " << err << LOG_END;
							if (disconn_cb_) disconn_cb_();
							if (auto_reconnect_)
							{
								{
									std::lock_guard<std::recursive_mutex> lock(mutex_);
									if (netport_) netport_->clear();
								}
								i_reconnect();
							}
						});

						netport_->bind_socket(sock);
						if(conn_cb_) conn_cb_(0);
					}
				}, NETWORK_PARAM::NP_TIMEOUT);
			}

			void stop()
			{
				std::lock_guard<std::recursive_mutex> lock(mutex_);

				auto_reconnect_ = false;
				stopped_	= true;
				conn_cb_	= {};
				disconn_cb_ = {};
				recv_cb_	= {};
				send_cb_	= {};

				if (netport_) netport_->clear();
			}

			void set_connect_callback(connect_callback cb)
			{
				conn_cb_ = cb;
			}

			void set_disconnect_callback(disconnect_callback cb)
			{
				disconn_cb_ = cb;
			}

			void set_recv_callback(recv_callback cb)
			{
				recv_cb_ = cb;
			}

			void set_send_callback(send_callback cb)
			{
				send_cb_ = cb;
			}

			void send_message(const std::string& buf, int len)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				if (netport_) netport_->send_message(buf, len);
			}

			void send_message(const char* data, int len)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				if (netport_) netport_->send_message(data, len);
			}

			void set_auto_reconnect(bool b, int interv = NP_RETRY_INTERVAL)
			{
				auto_reconnect_ = b;
				interval_ = interv;
			}

		private:
			void i_reconnect()
			{
				std::shared_ptr<timer_t> timer = std::make_shared<timer_t>();
				auto retain_this = shared_from_this();
				timer->wait([retain_this, this]() {
					start(ip_.c_str(), port_);
				}, interval_);
			}

		private:
			std::string						ip_;
			unsigned short					port_;
			standard_header*				std_hdr_;
			std::shared_ptr<connector_t>	connector_;			//	连接对象
			std::shared_ptr<netport_t>		netport_;			//	netport对象
			std::shared_ptr<strand_t>		strand_;			//	strand对象

			connect_callback				conn_cb_;
			send_callback					send_cb_;
			recv_callback					recv_cb_;
			disconnect_callback				disconn_cb_;

			bool							auto_reconnect_;	//	自动重连标志
			int								interval_;			//	重连间隔
			bool							stopped_;
			std::recursive_mutex			mutex_;
		};
	}
}

#endif

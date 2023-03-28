#ifndef __LISTENER_HELPER_HPP_2016_12_22__
#define __LISTENER_HELPER_HPP_2016_12_22__
#include <bas/socket.hpp>
#include <bas/netport.h>
#include <bas/timer.hpp>
#include <protocol/protocol.hpp>
#include <memory>

namespace bas
{
	namespace detail
	{
		struct listener_helper_t :
			std::enable_shared_from_this<listener_helper_t>
		{
		public:
			listener_helper_t(standard_header* hdr) : std_hdr_(hdr), stopped_(false) {}
			~listener_helper_t() {}

		public:
			void set_strand(std::shared_ptr<strand_t> strand)
			{
				strand_ = strand;
			}

			bool init(std::shared_ptr<socket_t> sock)
			{
				netport_ = std::make_unique<netport_t>(std_hdr_);
				netport_->set_strand(strand_);

				auto shared_this = shared_from_this();
				netport_->set_error_callback([shared_this, this](int err) {
					decltype(disconn_cb_) disconn_cb;
					{
						std::lock_guard<std::recursive_mutex> lock(mtx_);
						disconn_cb = disconn_cb_;
					}
					if (disconn_cb) disconn_cb(err);
				});
				netport_->set_recv_callback([shared_this, this]
					(const char* hdr, const char* body, int body_len) {
					decltype(recv_cb_) recv_cb;
					{
						std::lock_guard<std::recursive_mutex> lock(mtx_);
						recv_cb = recv_cb_;
					}
					if (recv_cb) recv_cb(hdr, body, body_len);
				});
				netport_->set_send_callback([shared_this](int){});

				netport_->bind_socket(sock);

				return true;
			}

			bool uninit()
			{
				std::lock_guard<std::recursive_mutex> lock(mtx_);
				stopped_ = true;
				recv_cb_ = {};
				disconn_cb_ = {};
				if (netport_) netport_->clear();
				return true;
			}

			void set_recv_callback(netport_t::recv_callback cb)
			{
				recv_cb_ = cb;
			}

			void set_disconnect_callback(netport_t::error_callback cb)
			{
				disconn_cb_ = cb;
			}

			void send_message(const char* data, int len)
			{
				std::lock_guard<std::recursive_mutex> lock(mtx_);
				if (netport_ && !stopped_) netport_->send_message(data, len);
			}

		private:
			bool							stopped_;
			std::recursive_mutex			mtx_;
			standard_header*				std_hdr_;
			std::shared_ptr<netport_t>		netport_;
			std::shared_ptr<strand_t>		strand_;
			netport_t::recv_callback		recv_cb_;
			netport_t::error_callback		disconn_cb_;
		};
	}
}

#endif

#pragma once

#include <boost/asio.hpp>
#include <string>
#include <istream>
#include "connection.hh"
#include "RequestHandler.hh"
#include "request.hh"
#include "response.hh"
#include "connection.hh"
#include "TcpConnection.hh"
#include "SslConnection.hh"
#include "ThreadPool.hh"

class Server {
public:
	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	explicit Server(boost::asio::io_service& service,
			const std::string& http_port = "",
			const std::string& https_port = "",
			size_t thread_pool_size = 10);

	void run(size_t thread_number = 1);

	void addHandler(const std::string& path, RequestHandlerPtr handle) {
		request_handler_.addSubHandler(path, handle);
	}

	void deliverRequest(RequestPtr req, ResponsePtr rep) {
		request_handler_.handleRequest(req, rep);
	}

	boost::asio::io_service& service() {
		return service_;
	}
	
	void post(const std::function<void(void)>& func) {
		service_.post(func);
	}
	
	template<typename _fCallable, typename... _tParams>
	auto enqueue(_fCallable&& f, _tParams&&... args) {
		return thread_pool_.enqueue(std::forward<_fCallable>(f), std::forward<_tParams>(args)...);
	}

private:
	boost::asio::io_service& service_;

	boost::asio::signal_set signals_;

	boost::asio::ip::tcp::socket socket_;

	boost::asio::ip::tcp::acceptor tcp_acceptor_;

	boost::asio::ip::tcp::acceptor ssl_acceptor_;

	RequestHandler request_handler_;

	TcpConnectionPtr new_tcp_connection_;

	SslConnectionPtr new_ssl_connection_;

	boost::asio::ssl::context ssl_context_;

	size_t thread_pool_size_;
	
	ThreadPool thread_pool_;

	void startAccept();

	void do_await_stop();

	void handleTcpAccept(const boost::system::error_code& ec);

	void handleSslAccept(const boost::system::error_code& ec);
};

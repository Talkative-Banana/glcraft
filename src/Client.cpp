#include "Client.h"

Client::Client(boost::asio::io_context &io_context, const std::string &host,
               const std::string &port,
               std::function<void(const std::string &)> func)
    : m_socket(io_context, udp::endpoint(udp::v4(), 0)), m_resolver(io_context),
      m_host(host), m_port(port), m_func(func) {}

void Client::run() { resolve(); }

void Client::resolve() {
  m_resolver.async_resolve(
      udp::v4(), m_host, m_port,
      [this](boost::system::error_code ec, udp::resolver::results_type result) {
        on_resolve(ec, result);
      });
}

void Client::on_resolve(boost::system::error_code ec,
                        udp::resolver::results_type result) {
  if (ec) {
    std::cerr << "fail on resolve: " << ec.message() << '\n';
    return;
  }

  if (result.empty()) {
    std::cerr << "resolve returned no endpoints\n";
    return;
  }

  std::cout << "Resolved endpoint\n";
  m_endpoint = *result.begin();
  on_recv();
}

void Client::on_recv() {
  m_socket.async_receive_from(
      boost::asio::buffer(m_recvdata, max_length), m_sender_endpoint,
      [this](boost::system::error_code ec, std::size_t bytes) {
        if (ec) {
          std::cerr << "fail to recv: " << ec.what() << '\n';
          return;
        }
        m_func(std::string(m_recvdata, bytes));
        on_recv();
      });
}

void Client::send(std::shared_ptr<std::string> msg) {
  m_socket.async_send_to(
      boost::asio::buffer(*msg), m_endpoint,
      [msg, this](boost::system::error_code ec, std::size_t bytes) {
        if (ec) {
          std::cerr << "fail to send: " << ec.what() << '\n';
          return;
        }
      });
}

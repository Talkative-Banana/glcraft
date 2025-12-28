#pragma once
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>

using boost::asio::ip::udp;

class Client {
public:
  Client(boost::asio::io_context &, const std::string &, const std::string &,
         std::function<void(const std::string &)>);

  void run();
  void resolve();
  void on_recv();
  void send(std::shared_ptr<std::string>);
  void on_resolve(boost::system::error_code, udp::resolver::results_type);

private:
  udp::socket m_socket;
  udp::resolver m_resolver;
  udp::endpoint m_endpoint;
  udp::endpoint m_sender_endpoint;
  const std::string m_host, m_port;
  enum { max_length = 1024 };
  char m_recvdata[max_length];
  std::function<void(const std::string &)> m_func;
};

#pragma once
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>

using boost::asio::ip::udp;

extern std::vector<std::string> world_operations;
struct EndpointCompare {
  bool operator()(const std::shared_ptr<udp::endpoint> &a,
                  const std::shared_ptr<udp::endpoint> &b) const {
    if (a->address() < b->address())
      return true;
    if (a->address() > b->address())
      return false;
    return a->port() < b->port();
  }
};

class Server : public std::enable_shared_from_this<Server> {
public:
  Server(boost::asio::io_context &io_context, short port);

  void send(std::shared_ptr<std::string>);
  void on_recv(
      std::function<std::shared_ptr<std::string>(const std::string &)> func);

private:
  udp::socket m_socket;
  enum { max_length = 1024 };
  char m_data[max_length];
  boost::asio::io_context &m_io_context;
  std::set<std::shared_ptr<udp::endpoint>, EndpointCompare> m_sender_endpoints;
};

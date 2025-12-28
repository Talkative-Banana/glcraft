#include "Server.h"

Server::Server(boost::asio::io_context &io_context, short port)
    : m_socket(io_context, udp::endpoint(udp::v4(), port)),
      m_io_context(io_context) {}

void Server::on_recv(
    std::function<std::shared_ptr<std::string>(const std::string &)> func) {

  std::shared_ptr<udp::endpoint> endpoint = std::make_shared<udp::endpoint>();
  m_socket.async_receive_from(
      boost::asio::buffer(m_data, max_length), *endpoint,
      [self = shared_from_this(), func, endpoint](boost::system::error_code ec,
                                                  std::size_t bytes_recvd) {
        // if a new client add to clientlist
        if (self->m_sender_endpoints.find(endpoint) ==
            self->m_sender_endpoints.end()) {
          std::cout << "Added new client " << endpoint->address() << ' '
                    << endpoint->port() << '\n';
          self->m_sender_endpoints.insert(endpoint);
        }

        if (!ec && bytes_recvd > 0) {
          std::shared_ptr<std::string> reply =
              func(std::string(self->m_data, bytes_recvd));
          self->send(reply);
        }
        self->on_recv(func);
      });
}

void Server::send(std::shared_ptr<std::string> msg) {
  auto handler = [self = shared_from_this(), msg](boost::system::error_code ec,
                                                  std::size_t bytes_sent) {};
  for (auto &endpoint : m_sender_endpoints) {
    // send player location to all clients
    m_socket.async_send_to(boost::asio::buffer(*msg), *endpoint, handler);
  }
}

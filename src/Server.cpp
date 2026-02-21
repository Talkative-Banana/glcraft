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
        bool new_client = self->m_sender_endpoints.find(endpoint) ==
                          self->m_sender_endpoints.end();
        // if a new client add to clientlist
        if (new_client) {
          std::cout << "Added new client " << endpoint->address() << ' '
                    << endpoint->port() << '\n';
          self->m_sender_endpoints.insert(endpoint);
        }

        if (!ec && bytes_recvd > 0) {
          std::shared_ptr<std::string> reply =
              func(std::string(self->m_data, bytes_recvd));
          self->send(reply);
          if (new_client) { // Replay previous operations to new_clients
            for (auto op : world_operations) {
              std::shared_ptr<std::string> msg =
                  std::make_shared<std::string>(op);
              self->send(msg);
            }
          }
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

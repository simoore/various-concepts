#include "boost/asio.hpp"
#include <iostream>

using namespace boost::asio;

namespace tcpechoserver {

std::string read(ip::tcp::socket &socket) {
    streambuf buf;
    read_until(socket, buf, "\n");
    std::string data = buffer_cast<const char *>(buf.data());
    return data;
}

void send(ip::tcp::socket &socket, const std::string &message) {
    const std::string msg = message + "\n";
    write(socket, buffer(msg));
}

};

int main() {

    io_context io;

    auto endpoint = ip::tcp::endpoint(ip::tcp::v4(), 12345);

    // The acceptor listens for a connection.
    boost::asio::ip::tcp::acceptor acceptor(io, endpoint);

    // Create socket.
    ip::tcp::socket sock(io);

    // Wait for a connection.
    acceptor.accept(sock);

    // read message.
    std::string msg = tcpechoserver::read(sock);
    std::cout << msg << std::endl;

    // write operation
    tcpechoserver::send(sock, "Hello From Server!");
    std::cout << "Server sent hello message to client" << std::endl;

    return 0;
}

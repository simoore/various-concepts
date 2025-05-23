#include <boost/asio.hpp>
#include "spdlog/fmt/fmt.h"

namespace tcpechoclient {

void recv(boost::asio::ip::tcp::socket &socket) {
    boost::asio::streambuf receiveBuffer;
    boost::system::error_code error;
    read(socket, receiveBuffer, boost::asio::transfer_all(), error);
    if (error && error != boost::asio::error::eof) {
        fmt::print("receive failed: {}\n", error.message());
    } else {
        const std::string data = reinterpret_cast<const char*>(receiveBuffer.data().data());
        fmt::println("{}\n", data);
    }
}

void send(boost::asio::ip::tcp::socket &socket, const std::string &msg) {
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer(msg), error);
    if (!error) {
        fmt::println("Client sent hello message!\n");
    } else {
        fmt::println("send failed: {}\n", error.message());
    }
}

}; // namespace tcpechoclient

int main() {
    
    // execution context
    boost::asio::io_context io;

    // socket creation
    boost::asio::ip::tcp::socket socket(io);

    // wait for connection
    socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address_v4("127.0.0.1"), 12345));

    // request/message from client
    const std::string msg = "Hello from Client!\n";
    tcpechoclient::send(socket, msg);
    
    // getting response from server
    tcpechoclient::recv(socket);

    return 0;
}
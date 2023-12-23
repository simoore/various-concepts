#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

namespace tcpechoclient {

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

    // socket creation
    ip::tcp::socket socket(io);

    // connection
    socket.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 12345));

    // request/message from client
    const std::string msg = "Hello from Client!\n";
    boost::system::error_code error;
    write(socket, boost::asio::buffer(msg), error);
    if (!error) {
        std::cout << "Client sent hello message!" << std::endl;
    } else {
        std::cout << "send failed: " << error.message() << std::endl;
    }

    // getting response from server
    streambuf receiveBuffer;
    read(socket, receiveBuffer, boost::asio::transfer_all(), error);
    if (error && error != boost::asio::error::eof) {
        std::cout << "receive failed: " << error.message() << std::endl;
    } else {
        const char* data = boost::asio::buffer_cast<const char*>(receiveBuffer.data());
        std::cout << data << std::endl;
    }
    return 0;
    
}
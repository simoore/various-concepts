///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In this example
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "boost/asio.hpp"

void print(const boost::system::error_code &) {
    std::cout << "Hello from asynchronouns timer" << std::endl;
}

int main() {
    boost::asio::io_context io;

    // using a timer asynchronously
    boost::asio::steady_timer timer2(io, boost::asio::chrono::seconds(5));
    timer2.async_wait(print);
    io.run();

    // using a timer synchronously
    boost::asio::steady_timer timer(io, boost::asio::chrono::seconds(5));
    timer.wait();
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In this example we demonstrate asynchronous and synchrous use ot boost asio timers.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/asio.hpp>
#include "spdlog/fmt/fmt.h"

void print(const boost::system::error_code &) {
    fmt::print("Hello from asynchronouns timer\n");
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
    fmt::print("Hello, world!\n");
    return 0;
}
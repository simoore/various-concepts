#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <print>

using default_token = boost::asio::as_tuple_t<boost::asio::use_awaitable_t<>>;
using tcp_acceptor = default_token::as_default_on_t<boost::asio::ip::tcp::acceptor>;
using tcp_socket = default_token::as_default_on_t<boost::asio::ip::tcp::socket>;

boost::asio::awaitable<void> echo(tcp_socket socket) {
    try {
        char data[1024];
        auto [eread, nread] = co_await socket.async_read_some(boost::asio::buffer(data));
        if (eread) {
            std::println("Read error {}", eread.message());
            co_return;
        } else if (nread == 0) {
            co_return;
        }
        std::println("Received from client: {}", std::string(data, nread));
        auto [ewrite, nwrite] = co_await socket.async_write_some(boost::asio::buffer(data, nread));
        if (ewrite) {
            std::println("write error {}", ewrite.message());
            co_return;
        } else if (nwrite != nread) {
            std::println("Didn't write enough characters: {}", nwrite);
            co_return;
        }
        std::println("Server Echoed");
    } catch (std::exception &e) {
        std::println("echo Exception: {}", e.what());
    }
}

boost::asio::awaitable<void> listener() {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor(executor, {boost::asio::ip::make_address_v4("127.0.0.1"), 12345});
    while (true) {
        tcp_socket socket = co_await acceptor.async_accept();
        boost::asio::co_spawn(executor, echo(std::move(socket)), boost::asio::detached);
    }
}

int main() {
    try {
        boost::asio::io_context io_context(1);

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ io_context.stop(); });

        boost::asio::co_spawn(io_context, listener(), boost::asio::detached);

        io_context.run();
    } catch (std::exception &e) {
        std::println("Exception: {}", e.what());
    }
}
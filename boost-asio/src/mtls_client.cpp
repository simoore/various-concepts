#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "spdlog/spdlog.h"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;

static asio::awaitable<void> run_client() {
    auto executor = co_await asio::this_coro::executor;
    ssl::context ssl_context(ssl::context::tls_client);

    ssl_context.load_verify_file("../../certificates/artifacts/smooreca.pem");
    ssl_context.use_certificate_file("../../certificates/artifacts/client.crt", ssl::context::pem);
    ssl_context.use_private_key_file("../../certificates/artifacts/client.key", ssl::context::pem);
    ssl_context.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

    asio::ip::tcp::resolver resolver(executor);
    auto endpoints = co_await resolver.async_resolve("127.0.0.1", "4433", asio::use_awaitable);

    ssl::stream<asio::ip::tcp::socket> socket(executor, ssl_context);

    co_await asio::async_connect(socket.lowest_layer(), endpoints, asio::use_awaitable);
    co_await socket.async_handshake(ssl::stream_base::client, asio::use_awaitable);

    std::string message = "Hello from mTLS client!";
    co_await boost::asio::async_write(socket, boost::asio::buffer(message), asio::use_awaitable);

    std::array<char, 1024> reply;
    size_t reply_length = co_await socket.async_read_some(boost::asio::buffer(reply), asio::use_awaitable);
    SPDLOG_INFO("Reply from server: {}", std::string(reply.data(), reply_length));

    co_await socket.async_shutdown(asio::use_awaitable);
}

static void client_complete_handler(std::exception_ptr e) {
    SPDLOG_INFO("Client has finished");
    try {
        if (e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception: {}", e.what());
    }
}

int main() {
    asio::io_context io_context;
    asio::co_spawn(io_context, run_client(), client_complete_handler);
    io_context.run();
    return 0;
}
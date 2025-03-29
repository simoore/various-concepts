#include <array>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "spdlog/spdlog.h"

namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;

/*****************************************************************************/
/********** SERVER ***********************************************************/
/*****************************************************************************/

/// This is a TCP with TLS echo client.
///
/// @param socket
///     The TLS socket to communicate with the client.
static asio::awaitable<void> handle_client(ssl::stream<asio::ip::tcp::socket> socket) {

    static constexpr auto token = asio::as_tuple(asio::use_awaitable);
    co_await socket.async_handshake(ssl::stream_base::server, asio::use_awaitable);

    std::array<char, 1024> data;
    while (true) {
        auto [ec, bytes] = co_await socket.async_read_some(boost::asio::buffer(data), token);

        if (ec) {
            if (ec != asio::error::eof) {
                SPDLOG_ERROR("Read error: {}", ec.message());
            }
            break;
        }

        SPDLOG_INFO("Received: {}", std::string(data.data(), bytes));
        co_await asio::async_write(socket, boost::asio::buffer(data, bytes), asio::use_awaitable);
    }

    auto [ec] = co_await socket.async_shutdown(token);
    if (ec) {
        SPDLOG_ERROR("Error shutting down: {}", ec.message());
    }
}

/// The handle client completion handle simply logs any thrown exceptions.
///
/// @param e 
///     Any exception thrown during the task.
static void handle_client_complete_handler(std::exception_ptr e) {
    SPDLOG_INFO("Handle client has terminated");
    try {
        if (e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Handle client error: {}", e.what());
    }
}

/// Launches the server by loading in the TLS certificates, and launching the listening socket.
static asio::awaitable<void> run_server() {
    auto executor = co_await asio::this_coro::executor;
    ssl::context ssl_context(ssl::context::tls_server);

    ssl_context.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2);
    ssl_context.use_certificate_file("../../certificates/artifacts/server.crt", ssl::context::pem);
    ssl_context.use_private_key_file("../../certificates/artifacts/server.key", ssl::context::pem);
    ssl_context.load_verify_file("../../certificates/artifacts/smooreca.pem");
    ssl_context.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

    SPDLOG_INFO("mTLS Server running on port 4433...");
    asio::ip::tcp::acceptor acceptor(executor, {asio::ip::tcp::v4(), 4433});

    while (true) {
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        ssl::stream<asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_context);
        asio::co_spawn(executor, handle_client(std::move(ssl_socket)), handle_client_complete_handler);
    }
}

/// Handle the completion of the server task.
///
/// @param e 
///     An exception if one was thrown during the server task.
static void server_complete_handler(std::exception_ptr e) {
    SPDLOG_INFO("Server has terminated");
    try {
        if (e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Server error: {}", e.what());
    }
}

/*****************************************************************************/
/********** MAIN *************************************************************/
/*****************************************************************************/

int main() {
    asio::io_context io_context; 
    asio::co_spawn(io_context, run_server(), server_complete_handler);
    io_context.run();
    return 0;
}
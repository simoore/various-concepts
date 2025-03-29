#include <boost/asio/awaitable.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/config.hpp>
#include <map>

#include "request_handler.h"
#include "spdlog/spdlog.h"

/*****************************************************************************/

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;
namespace ssl   = asio::ssl;

static uint32_t s_task_counter = 0;
static std::map<uint32_t, asio::cancellation_signal> s_cancellation_signals;

/*****************************************************************************/

/// This function gets called when the task completes.
/// 
/// @param id
///     The id of the task to identify its cancellation signal and remove it.
/// @param e
///     A raised exception in the task if there was one.
static void task_completion_handler(uint32_t id, std::exception_ptr e) {
    SPDLOG_INFO("Task {} has completed", id);
    s_cancellation_signals.erase(id);
    try {
        if (e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Task error: {}", e.what());
    }
}

/// This launches a new detached task and registers it so it can be cancelled later.
///
/// @param ioc 
///     The execution context.
/// @param awaitable 
///     The awaitable to launch.
static void launch_new_task(auto &ioc, auto &&awaitable) {
    uint32_t id = s_task_counter;
    s_cancellation_signals.emplace(std::piecewise_construct, std::make_tuple(id), std::make_tuple());
    asio::cancellation_signal cs;
    auto token = asio::bind_cancellation_slot(
        s_cancellation_signals.at(id).slot(), 
        [id](std::exception_ptr e) { task_completion_handler(id, e); }
    );
    asio::co_spawn(ioc, std::move(awaitable), token);
    s_task_counter++;
}

/// Handles an HTTP server connection.
///
/// @param stream
///     The TCP stream from the client.
/// @param doc_root
///     The directory containing the files to serve.
/// @param buffer
///     Buffer that is used to read network traffic and buffer the request.
static asio::awaitable<void> handle_session(
    ssl::stream<beast::tcp_stream> &stream, 
    const std::string_view doc_root,
    beast::flat_buffer &buffer
) {
    auto cs = co_await asio::this_coro::cancellation_state;

    while (!cs.cancelled()) {

        // Read a request.
        Request req;
        auto [ec, _] = co_await http::async_read(stream, buffer, req, asio::as_tuple(asio::use_awaitable));

        if (ec) {
            if (ec != http::error::end_of_stream) {
                SPDLOG_ERROR("Read error: {} {}", ec.value(), ec.message());
            }
            break;
        }

        // Handle the request.
        http::message_generator msg = handle_request(doc_root, std::move(req));

        // Determine if we should close the connection. It suggests another request is coming.
        bool keep_alive = msg.keep_alive();

        // Send the response
        co_await beast::async_write(stream, std::move(msg), asio::use_awaitable);

        if (!keep_alive) {
            // This means we should close the connection, usually because the response indicated the 
            // "Connection: close" semantic.
            break;
        }
    }
}

/// Detects if the session is using TLS and if so launches the session.
///
/// @param stream
///     The SSL socket to transfer data. 
/// @param ctx 
///     The SSL context.
/// @param doc_root 
///     The location of the rsources available to transmit.
static asio::awaitable<void> do_session(
    beast::tcp_stream stream,
    ssl::context &ctx,
    std::string_view doc_root
) {
    beast::flat_buffer buffer;

    // Allow total cancellation to change the cancellation state of this coroutine, but only allow terminal 
    // cancellation to propagate to async operations. This setting will be inherited by all child coroutines.
    co_await asio::this_coro::reset_cancellation_state(
        asio::enable_total_cancellation(), 
        asio::enable_terminal_cancellation()
    );

    // We want to be able to continue performing new async operations, such as cleanups, even after the coroutine is 
    // cancelled. This setting will be inherited by all child coroutines.
    co_await asio::this_coro::throw_if_cancelled(false);

    stream.expires_after(std::chrono::seconds(30));

    auto [ecc, res] = co_await beast::async_detect_ssl(stream, buffer, asio::as_tuple(asio::use_awaitable));
    if (ecc) {
        SPDLOG_ERROR("Async detect ssl error: {}", ecc.message());
        co_return;
    } else if (!res) {
        if (stream.socket().is_open()) {
            stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
        }
        SPDLOG_INFO("Shutting down non-ssl socket..");  
        co_return;
    }

    ssl::stream<beast::tcp_stream> ssl_stream(std::move(stream), ctx);

    SPDLOG_INFO("Starting SSL handshake");
    auto bytes_transferred = co_await ssl_stream.async_handshake(
        ssl::stream_base::server, 
        buffer.data(), 
        asio::use_awaitable
    );

    buffer.consume(bytes_transferred);

    co_await handle_session(ssl_stream, doc_root, buffer);

    if (!ssl_stream.lowest_layer().is_open()) {
        SPDLOG_INFO("SSL stream is closed.");
        co_return;
    }

    // Gracefully close the stream
    auto [ec] = co_await ssl_stream.async_shutdown(asio::as_tuple(asio::use_awaitable));
    if (ec && ec != ssl::error::stream_truncated) {
        throw boost::system::system_error(ec);
    }
    SPDLOG_INFO("Exiting session.");
}

/// Accepts incoming connections and launches the sessions.
///
/// @param endpoint
///     The endpoint for the listening socket to bind to.
/// @param doc_root
///     The location of the root directory for the resources to serve.
/// @param ctx
///     The SSL context.
static asio::awaitable<void> do_listen(
    asio::ip::tcp::endpoint endpoint, 
    const std::string_view doc_root,
    ssl::context &ctx
) {
    auto cs = co_await asio::this_coro::cancellation_state;
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, endpoint);

    // Allows all cancelation signals to propagate.
    co_await asio::this_coro::reset_cancellation_state(
        asio::enable_total_cancellation()
    );

    while (!cs.cancelled()) {
        SPDLOG_INFO("Waiting for connection");
        auto [ec, socket] = co_await acceptor.async_accept(asio::as_tuple(asio::use_awaitable));
        if (ec == asio::error::operation_aborted) {
            SPDLOG_INFO("Acceptor operation aborted");
            co_return;
        }
        auto do_session_awaitable = do_session(beast::tcp_stream(std::move(socket)), ctx, doc_root);
        launch_new_task(executor, do_session_awaitable);
    }
    SPDLOG_INFO("Exiting listening task.");
}

/// In this function we wait for the SIGINT or SIGTERM and we cancel all tasks and terminate. We catch these signals
/// since we need to properly terminate a TLS connection before we close.
static asio::awaitable<void> handle_signals() {

    auto executor = co_await asio::this_coro::executor;
    auto signal_set = asio::signal_set(executor, SIGINT, SIGTERM);
    auto sig = co_await signal_set.async_wait(asio::use_awaitable);\
    asio::system_timer timer(executor); 
    
    if (sig == SIGINT) {
        SPDLOG_INFO("Handling SIGINT");
        for (auto &kv : s_cancellation_signals) {
            kv.second.emit(asio::cancellation_type::total);
        }
        
        // Poll termination flag. You could create a custom awaitable to poll for you.
        int poll_count = 0;
        bool terminate = false;
        while (true) {
            if (s_cancellation_signals.empty()) {
                break;
            }
            if (poll_count == 20) {
                terminate = true;
                break;
            }
            timer.expires_after(std::chrono::milliseconds(200));
            co_await timer.async_wait(asio::use_awaitable);
            poll_count++;
        }

        if (terminate) {
            SPDLOG_INFO("Emitting termination signal");
            for (auto &kv : s_cancellation_signals) {
                kv.second.emit(asio::cancellation_type::terminal);
            }
        }

        SPDLOG_INFO("Finished handling SIGINT.");
    } else {
        SPDLOG_INFO("Handling SIGTERM.");
        for (auto &kv : s_cancellation_signals) {
            kv.second.emit(asio::cancellation_type::terminal);
        }
    }
}

/*****************************************************************************/
/********** MAIN FUNCTION ****************************************************/
/*****************************************************************************/

int main() {
    const auto address  = asio::ip::make_address("127.0.0.1");
    const uint16_t port = 7778;
    const std::string doc_root("../resources");
    const std::string cert_root("../../certificates/artifacts/");

    SPDLOG_INFO("doc_root is: {}", doc_root);

    // The io_context is required for all I/O
    asio::io_context ioc;

    // The SSL context holds certificates.
    ssl::context ssl_context(ssl::context::tls_server);
    ssl_context.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2);
    ssl_context.use_certificate_file(cert_root + "server.crt", ssl::context::pem);
    ssl_context.use_private_key_file(cert_root + "server.key", ssl::context::pem);
    ssl_context.load_verify_file(cert_root + "smooreca.pem");
    ssl_context.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

    // The endpoint to bind the listening socket to.
    auto listening_endpoint = asio::ip::tcp::endpoint{address, port};
    auto do_listen_awaitable = do_listen(listening_endpoint, doc_root, ssl_context);
    launch_new_task(ioc, do_listen_awaitable);

    // It is detached because we expect no return values.
    asio::co_spawn(ioc, handle_signals(), asio::detached);

    SPDLOG_INFO("Service starting.");
    ioc.run();
    SPDLOG_INFO("Service exiting");

    return 0;
}

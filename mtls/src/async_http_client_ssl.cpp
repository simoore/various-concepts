#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>

#include "spdlog/spdlog.h"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;
namespace ssl   = boost::asio::ssl;

/// Performs an HTTP GET and prints the response body.
///
/// @param host
///     The host name or IP address eg. "127.0.0.1"
/// @param port
///     The server port, eg. 7788
/// @param target
///     The HTTP target resource to fetch.
/// @param version
///     The HTTP version to use.
/// @param ctx
///     The SSL/TLS context for this application.
asio::awaitable<void> do_session(
    std::string host,
    std::string port,
    std::string target,
    int version,
    ssl::context& ctx
) {
    auto executor = co_await asio::this_coro::executor;
    auto resolver = asio::ip::tcp::resolver(executor);
    auto stream   = ssl::stream<beast::tcp_stream>(executor, ctx);

    // Look up the domain name
    auto const results = co_await resolver.async_resolve(host, port, asio::use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    co_await beast::get_lowest_layer(stream).async_connect(results, asio::use_awaitable);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    co_await stream.async_handshake(ssl::stream_base::client, asio::use_awaitable);

    // Set up an HTTP GET request message
    http::request<http::string_body> req(http::verb::get, target, version);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, req, asio::use_awaitable);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    co_await http::async_read(stream, buffer, res, asio::use_awaitable);

    // Write the message to standard out
    std::stringstream ss;
    ss << res;
    SPDLOG_INFO("Read res {}", ss.view());

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream - do not threat every error as an exception!
    auto [ec] = co_await stream.async_shutdown(asio::as_tuple(asio::use_awaitable));

    // ssl::error::stream_truncated, also known as an SSL "short read", indicates the peer closed the connection 
    // without performing the required closing handshake (for example, Google does this to improve performance). 
    // Generally this can be a security issue, but if your communication protocol is self-terminated (as it is with 
    // both HTTP and WebSocket) then you may simply ignore the lack of close_notify.
    //
    // https://github.com/boostorg/beast/issues/38
    //
    // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
    //
    // When a short read would cut off the end of an HTTP message, Beast returns the error 
    // beast::http::error::partial_message. Therefore, if we see a short read here, it has occurred after the message 
    // has been completed, so it is safe to ignore it.

    if(ec && ec != asio::ssl::error::stream_truncated) {
        throw boost::system::system_error(ec, "shutdown");
    }
}

/// The session completion handler.
///
/// @param e 
///     Points to an exception if one was thrown during the task.
static void session_complete(std::exception_ptr e) {
    SPDLOG_INFO("Session has ended");
    try {
        if(e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Session error {}", e.what());
    }
}

/*****************************************************************************/
/********** MAIN *************************************************************/
/*****************************************************************************/

int main() {
    try {
        static constexpr std::string host   = "127.0.0.1";
        static constexpr std::string port   = "7778";
        static constexpr std::string target = "/";
        static constexpr int version        = 11;

        // The io_context is required for all I/O
        asio::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx(ssl::context::tls_client);
        ctx.load_verify_file("../../certificates/artifacts/smooreca.pem");
        ctx.use_certificate_file("../../certificates/artifacts/client.crt", ssl::context::pem);
        ctx.use_private_key_file("../../certificates/artifacts/client.key", ssl::context::pem);
        ctx.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

        // Launch the asynchronous operation
        auto session_awaitable = do_session(host, port, target, version, ctx);
        asio::co_spawn(ioc, std::move(session_awaitable), session_complete);

        ioc.run();
    } catch(std::exception const& e) {
        SPDLOG_ERROR("Error: {}", e.what()); 
    }
    return 0;
}

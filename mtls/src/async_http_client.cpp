#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
\
#include <iostream>
#include <string>

#include "spdlog/spdlog.h"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;

// Performs an HTTP GET and prints the response
asio::awaitable<void> do_session(std::string host, std::string port, std::string target, int version) {
    auto executor = co_await asio::this_coro::executor;
    auto resolver = asio::ip::tcp::resolver{executor};
    auto stream   = beast::tcp_stream{executor};

    // Look up the domain name
    auto const results = co_await resolver.async_resolve(host, port, asio::use_awaitable);

    // Set the timeout.
    stream.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    co_await stream.async_connect(results, asio::use_awaitable);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Set the timeout.
    stream.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, req, asio::use_awaitable);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    co_await http::async_read(stream, buffer, res, asio::use_awaitable);

    // Write the message to standard out
    std::cout << res << std::endl;

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes, so don't bother reporting it.
    if (ec && ec != beast::errc::not_connected) {
        throw boost::system::system_error(ec, "shutdown");
    }
}

//------------------------------------------------------------------------------

int main()
{
    try
    {
        static constexpr std::string host   = "www.example.com";
        static constexpr std::string port   = "80";
        static constexpr std::string target = "/";
        static constexpr int version        = 11;

        // The io_context is required for all I/O
        asio::io_context ioc;

        // If the awaitable exists with an exception, it gets delivered here as `e`. This can happen for regular 
        // errors, such as connection drops.
        auto error_handler = [](std::exception_ptr e) {
            if (e) {
                std::rethrow_exception(e);
            }
        };

        // Launch the asynchronous operation
        asio::co_spawn(ioc, do_session(host, port, target, version), error_handler);
            
        // Run the I/O service. The call will return when
        // the get operation is complete.
        ioc.run();

    } catch (const std::exception &e) {
        SPDLOG_ERROR("Error: {}", e.what());
        return 0;
    }
    return 0;
}

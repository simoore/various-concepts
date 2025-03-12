// Based on https://www.boost.org/doc/libs/1_87_0/libs/beast/doc/html/beast/quick_start/http_client.html

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

#include "spdlog/spdlog.h"

using Request = boost::beast::http::request<boost::beast::http::string_body>;

// Performs an HTTP GET and prints the response
int main()
{
    try
    {
        const std::string host("www.example.com");
        const std::string port("80");
        const std::string target("/");
        const uint32_t version = 11; // version 1.1

        // The io_context is required for all I/O
        boost::asio::io_context ioc;

        // These objects perform our I/O
        boost::asio::ip::tcp::resolver resolver(ioc);
        boost::beast::tcp_stream stream(ioc);

        // Look up the domain name
        const auto results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET request message
        Request req{boost::beast::http::verb::get, target, version};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        boost::beast::http::write(stream, req);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        // Receive the HTTP response
        boost::beast::http::read(stream, buffer, res);

        // Write the message to standard out
        std::cout << res << std::endl;

        // Gracefully close the socket
        boost::beast::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes, so don't bother reporting it.
        //
        if (ec && ec != boost::beast::errc::not_connected) {
            throw boost::beast::system_error{ec};
        }

        // If we get here then the connection is closed gracefully
    
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Error: {}", e.what());
        return 0;
    }
    return 0;
}
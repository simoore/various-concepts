#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

/*****************************************************************************/

namespace beast = boost::beast;
namespace http  = beast::http;
namespace asio  = boost::asio;

/*****************************************************************************/

template <typename Body, typename Allocator>
using Request = http::request<Body, http::basic_fields<Allocator>>;
using Response = http::response<http::string_body>;

/*****************************************************************************/

/// We only support GET and HEAD methods.
bool is_illegal_method(http::verb req_method) {
    return (req_method != http::verb::get) && (req_method != http::verb::head);
}

/// Targets must not be empty, must start with "/" and cannot contain ".."
bool is_illegal_target(const std::string_view req_target) {
    return req_target.empty() || req_target[0] != '/' || req_target.find("..") != beast::string_view::npos;
}

/// Returns a bad request response.
template <typename Body, typename Allocator>
Response bad_request(const Request<Body, Allocator> &req, const std::string_view why) {
    Response res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
};

/// Returns a not found response.
template <typename Body, typename Allocator>
Response not_found(const Request<Body, Allocator> &req, const std::filesystem::path &target) {
    Response res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + target.string() + "' was not found.";
    res.prepare_payload();
    return res;
};

// Returns a server error response
template <typename Body, typename Allocator>
Response server_error(const Request<Body, Allocator> &req, const std::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
};

/// Return a reasonable mime type based on the extension of a file.
std::string mime_type(const std::filesystem::path &path) {
    if (path.extension() == std::filesystem::path(".html")) {
        return "text/html";
    } else if (path.extension() == std::filesystem::path(".jpg")) {
        return "image/jpeg";
    } else {
        return "application/text";
    }
}

/// Return a response for the given request.
template <typename Body, typename Allocator>
http::message_generator handle_request(const std::string &doc_root, Request<Body, Allocator> &&req) {

    // Make sure we can handle the method
    if (is_illegal_method(req.method())) {
        return bad_request(req, "Unknown HTTP-method");
    }

    // Request path must be absolute and not contain "..".
    if (is_illegal_target(req.target())) {
        return bad_request(req, "Illegal request-target");
    }

    // Build the path to the requested file
    std::string target(req.target());
    std::filesystem::path filepath;
    if (req.target().back() == '/') {
        filepath = std::filesystem::canonical(doc_root + "/index.html");
    } else {
        filepath = std::filesystem::canonical(doc_root + target);
    }
    SPDLOG_INFO("Filepath request: {}", filepath.string());

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(filepath.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == beast::errc::no_such_file_or_directory) {
        return not_found(req, target);
    } else if (ec) {
        return server_error(req, ec.message());
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head) {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(filepath));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct, 
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())
    };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(filepath));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
}

/// Handles an HTTP server connection.
asio::awaitable<void> do_session(
    beast::tcp_stream stream, 
    std::shared_ptr<const std::string> doc_root
) {
    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    while (true) {

        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request.
        http::request<http::string_body> req;
        co_await http::async_read(stream, buffer, req, asio::use_awaitable);

        // Handle the request.
        http::message_generator msg = handle_request(*doc_root, std::move(req));

        // Determine if we should close the connection. It suggests another request is coming.
        bool keep_alive = msg.keep_alive();

        // Send the response
        co_await beast::async_write(stream, std::move(msg), asio::use_awaitable);

        if (!keep_alive) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
}

/// Accepts incoming connections and launches the sessions.
asio::awaitable<void> do_listen(
    asio::ip::tcp::endpoint endpoint, 
    std::shared_ptr<const std::string> doc_root
) {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor{executor, endpoint};

    auto error_handler = [](std::exception_ptr e) {
        try {
            if (e) {
                std::rethrow_exception(e);
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("Session error: {}", e.what());
        }
    };

    while (true) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        auto do_session_awaitable = do_session(beast::tcp_stream(std::move(socket)), doc_root);
        asio::co_spawn(executor, std::move(do_session_awaitable), error_handler);
    }
}

/*****************************************************************************/

int main() {

    const auto address  = asio::ip::make_address("127.0.0.1");
    const uint16_t port = 7778;
    auto doc_root = std::make_shared<const std::string>("../resources");
    const int threads = 3;

    SPDLOG_INFO("doc_root is: {}", *doc_root);

    // The io_context is required for all I/O
    asio::io_context ioc{threads};

    // The endpoint to bind the listening socket to.
    auto listening_endpoint = asio::ip::tcp::endpoint{address, port};

    auto error_handler = [](std::exception_ptr e) {
        if (e) {
            std::rethrow_exception(e);
        }
    };

    // Spawn a listening port
    auto do_listen_awaitable = do_listen(listening_endpoint, doc_root);
    asio::co_spawn(ioc, std::move(do_listen_awaitable), error_handler);

    // Run the I/O service on the requested number of threads
    // We launch threads - 1 new threads because we use main thread as well.
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (int i = 0; i < threads - 1; ++i) {
        v.emplace_back([&ioc] { ioc.run(); });
    }
    ioc.run();

    return 0;
}

#include <boost/beast/version.hpp>
#include <filesystem>

#include "spdlog/spdlog.h"

#include "request_handler.h"

namespace beast = boost::beast;
namespace http  = beast::http;

using Response = http::response<http::string_body>;

/// We only support GET and HEAD methods.
static inline bool is_illegal_method(http::verb req_method) {
    return (req_method != http::verb::get) && (req_method != http::verb::head);
}

/// Targets must not be empty, must start with "/" and cannot contain ".."
static inline bool is_illegal_target(const std::string_view req_target) {
    return req_target.empty() || req_target[0] != '/' || req_target.find("..") != beast::string_view::npos;
}

/// Returns a bad request response.
static Response bad_request(const Request &req, const std::string_view why) {
    Response res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
};

/// Returns a not found response.
static Response not_found(const Request &req, const std::filesystem::path &target) {
    Response res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + target.string() + "' was not found.";
    res.prepare_payload();
    return res;
};

// Returns a server error response
static Response server_error(const Request &req, const std::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
};

/// Return a reasonable mime type based on the extension of a file.
static std::string mime_type(const std::filesystem::path &path) {
    if (path.extension() == std::filesystem::path(".html")) {
        return "text/html";
    } else if (path.extension() == std::filesystem::path(".jpg")) {
        return "image/jpeg";
    } else {
        return "application/text";
    }
}

/// Return a response for the given request.
http::message_generator handle_request(const std::string_view doc_root, Request &&req) {

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
        filepath = std::filesystem::canonical(std::string(doc_root) + "/index.html");
    } else {
        filepath = std::filesystem::canonical(std::string(doc_root) + target);
    }
    SPDLOG_INFO("Filepath request: {}", filepath.string());

    // Attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(filepath.c_str(), boost::beast::file_mode::scan, ec);

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

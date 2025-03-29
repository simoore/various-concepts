#pragma once

#include <boost/beast/http.hpp>
#include <string_view>

using Request = boost::beast::http::request<boost::beast::http::string_body>;

boost::beast::http::message_generator handle_request(const std::string_view doc_root, Request &&req);

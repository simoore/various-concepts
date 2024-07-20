#pragma once

#include <chrono>
#include <memory>

#include "spdlog/spdlog.h"
#include "zmq.hpp"

/// Encaptulates a ZeroMQ pub socket.
class Pub {
public:

    /// Sets the context and address of this socket.
    ///
    /// @param addr 
    ///
    /// @param context 
    ///     The zeromq execution context this socket runs in.
    Pub(std::string &&addr, std::shared_ptr<zmq::context_t> context):
        mAddr(std::move(addr)),
        mContext(context),
        mSocket()
    {}

    /// Binds a socket to a network address and opens the socket.
    ///
    /// @exception zmq::error_t
    ///     Raises this error if bind fails.
    void bind() {
        zmq::socket_t newSocket(*mContext, zmq::socket_type::pub);
        mSocket = std::move(newSocket);
        try {
            mSocket.bind(mAddr);
        } catch (const zmq::error_t &e) {
            spdlog::error("Pub bind error: {}", e.what());
            throw;
        }
    }

    /// Closes the socket.
    void close() {
        mSocket.close();
    }

    /// Sends a message on the socket. If the send fails, it attempts to recreate the socket.
    ///
    /// @param message 
    ///     The message to send.
    /// @exception zmq::error_t
    ///     If there is a send error. Depending on the error you may want to recreate the socket with bind()
    void send(std::string_view message) {
        using namespace std::chrono_literals;
        spdlog::info("Sending message: {}", message);
        try {
            mSocket.send(zmq::buffer(message), zmq::send_flags::none);
        } catch (const zmq::error_t &e) {
            spdlog::error("Pub send error: {}", e.what());
            throw;
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The zeromq address.
    std::string mAddr;

    /// The zeromq context used by this socket.
    std::shared_ptr<zmq::context_t> mContext;

    /// The zeromq context
    zmq::socket_t mSocket;

};
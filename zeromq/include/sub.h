#pragma once

#include <memory>
#include <zmq.hpp>

#include "spdlog/spdlog.h"


/// Encaptulates a ZeroMQ PUB socket.
class Sub {
public:

    /// Sets the context and address of this socket.
    ///
    /// @param addr 
    ///
    /// @param context 
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

    void close() {
        mSocket.close();
    }

    void send(std::string_view message) {
        spdlog::info("Sending message: {}", message);
        mSocket.send(zmq::buffer(message), zmq::send_flags::none);
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
#include <boost/asio.hpp>
#include <memory>
#include <print>
#include <string_view>

namespace asyncserver {

// enable_shared_from_this allows an object that is currently managed by a shared_ptr to safely generate 
// new shared pointers from the raw pointer this. Without this approach you can end up with multiple independant
// shared pointers that don't share a reference count and end up with dangling references.
class Server : public std::enable_shared_from_this<Server> {
private:

    /*************************************************************************/
    /********** PRIVATE TYPES ************************************************/
    /*************************************************************************/

    // Tag to make functions private.
    struct Private {};

public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    static constexpr std::string_view sMessage = "Hello From Server!";
    static constexpr size_t sMaxLength = 1024;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    Server(Private, boost::asio::io_context &io) : 
        mAcceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address_v4("127.0.0.1"), 12345)), 
        mSocket(io) 
    {}

    // Everyone has to use this factory function to create objects. Hence all created objects will be managed by 
    // shared pointers.
    static std::shared_ptr<Server> create(boost::asio::io_context &io) {
        return std::make_shared<Server>(Private(), io);
    }

    boost::asio::ip::tcp::socket &socket() {
        return mSocket;
    }

    void echo() {
        auto ptr = shared_from_this();

        auto writeToken = [ptr](const boost::system::error_code &err, size_t) {
            if (!err) {
                std::println("Server echoed!");
                // Can call async_read_some again.
            } else {
                std::println("error: {}", err.message());
                ptr->mSocket.close();
            }
        };

        auto readToken = [ptr, writeToken](const boost::system::error_code &err, size_t bytesTransferred) {
            if (!err) {
                std::string msg{ptr->mData, bytesTransferred};
                std::println("Server received: {}", msg);
                ptr->mSocket.async_write_some(boost::asio::buffer(ptr->mData, bytesTransferred), writeToken);
            } else {
                std::println("error: {}", err.message());
                ptr->mSocket.close();
            }
        };

        // See https://live.boost.org/doc/libs/1_83_0/doc/html/boost_asio/overview/model/completion_tokens.html
        // for a description of completion tokens which are used to communicate to an application the an async
        // operation has finished.
        mSocket.async_read_some(boost::asio::buffer(mData, sMaxLength), readToken);
    }

    void start() {
        auto ptr = shared_from_this();

        auto acceptToken = [ptr](const boost::system::error_code &err) {
            if (!err) {
                ptr->echo();
            }
            // Can call async_accept again to wait for another connection.
        };

        mAcceptor.async_accept(mSocket, acceptToken);
    }

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The object that listens for connections.
    boost::asio::ip::tcp::acceptor mAcceptor;

    // The socket for an active connection. socket.
    boost::asio::ip::tcp::socket mSocket;

    // This is the receive data buffer.
    char mData[sMaxLength];
};

} // namespace asyncserver

int main() {
    boost::asio::io_context io;
    auto server = asyncserver::Server::create(io);
    server->start();
    io.run();
}
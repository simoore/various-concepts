#include <iostream>
#include <memory>
#include <string_view>

#include "boost/asio.hpp"

namespace asyncserver {

using namespace boost::asio;

// enable_shared_from_this allows an object that is currently managed by a shared_ptr to safely generate 
// new shared pointers from the raw pointer this. Without this approach you can end up with multiple independant
// shared pointers that don't share a reference count and end up with dangling references.
class Connection : public std::enable_shared_from_this<Connection> {
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

    Connection(Private, io_context io) : mSocket(io) {}

    // Everyone has to use this factory function to create objects. Hence all created objects will be managed by 
    // shared pointers.
    static std::shared_ptr<Connection> create(io_context io) {
        return std::make_shared<Connection>(Private(), io);
    }

    ip::tcp::socket &socket(void) {
        return mSocket;
    }

    void start(void) {
        auto ptr = shared_from_this();

        auto readToken = [ptr](const boost::system::error_code& err, size_t bytes_transferred) {
            if (!err) {
                std::cout << ptr->mData << std::endl;
            } else {
                std::cout << "error: " << err.message() << std::endl;
            }
        };

        // See https://live.boost.org/doc/libs/1_83_0/doc/html/boost_asio/overview/model/completion_tokens.html
        // for a description of completion tokens which are used to communicate to an application the an async
        // operation has finished.
        mSocket.async_read_some(buffer(mData, sMaxLength), readToken);

        auto writeToken = [ptr](const boost::system::error_code& err, size_t bytes_transferred) {
            if (!err) {
                std::cout << "Server sent Hello message!"<< std::endl;
            } else {
                std::cerr << "error: " << err.message() << std::endl;
                ptr->mSocket.close();
            }
        };

        mSocket.async_write_some(buffer(sMessage, sMaxLength), writeToken);
    }

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    ip::tcp::socket mSocket;
    char mData[sMaxLength];

};

} // namespace 
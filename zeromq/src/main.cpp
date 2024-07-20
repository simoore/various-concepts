#include <string>
#include <chrono>
#include <thread>
#include <zmq.hpp>

#include "spdlog/spdlog.h"


#include "pub_thread.h"

int main() {

    using namespace std::chrono_literals;
    spdlog::info("Hello, {}!", "World");

    // initialize the zmq context with a single IO thread
    auto context = std::make_shared<zmq::context_t>(1);

    // construct a socket and bind to interface
    PubThread pubThread("tcp://*:5555", context);
    pubThread.start();


    // prepare some static data for responses
    std::string data{"World"};

    //zmq::message_t request;

    // receive a request from client
    // socket.recv(request, zmq::recv_flags::none);
    //std::cout << "Received " << request.to_string() << std::endl;
    pubThread.send(std::move(data));

    // simulate work
    std::this_thread::sleep_for(1s);
    spdlog::info("Pub thread alive: {}", pubThread.alive());

    Pub pubSocket2("tcp://*:5555", context);
    try {
        pubSocket2.bind();
    } catch (...) {

    }

    pubThread.stop();
    spdlog::info("Pub thread alive: {}", pubThread.alive());
    spdlog::info("Terminating application");
    return 0;
}
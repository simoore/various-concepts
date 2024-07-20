#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <chrono>

void rep_socket_thread(zmq::context_t& context, const std::string& endpoint) {
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind(endpoint);
    
    zmq::poller_t<> poller;
    poller.add(socket, zmq::event_flags::pollin);

    auto recreate_socket = [&context, &endpoint]() {
        zmq::socket_t new_socket(context, zmq::socket_type::rep);
        new_socket.bind(endpoint);
        return new_socket;
    };

    while (true) {
        try {
            // Poll for incoming messages
            auto events = poller.wait(std::chrono::milliseconds(1000));
            if (events.size() > 0 && events[0].socket == socket) {
                try {
                    zmq::message_t message;
                    socket.recv(message, zmq::recv_flags::dontwait);
                    std::cout << "Received message: " << message.to_string() << std::endl;
                    
                    // Process the message and send a reply
                    zmq::message_t reply("Reply");
                    socket.send(reply, zmq::send_flags::none);
                    std::cout << "Sent reply: Reply" << std::endl;
                } catch (const zmq::error_t& e) {
                    if (e.num() == EAGAIN) {
                        std::cout << "Transient error (EAGAIN) on recv: " << e.what() << std::endl;
                    } else if (e.num() == ETIMEDOUT) {
                        std::cout << "Transient error (ETIMEDOUT) on recv: " << e.what() << std::endl;
                    } else {
                        std::cout << "Non-transient error on recv (" << e.num() << "): " << e.what() << std::endl;
                    }
                }
            } else {
                std::cout << "No message received, continuing" << std::endl;
            }
        } catch (const zmq::error_t& e) {
            std::cout << "Error in poller: " << e.what() << std::endl;
            if (e.num() == ETERM || e.num() == ENOTSOCK || e.num() == EFSM) {
                // Handle termination, invalid socket, or incorrect state
                std::cout << "Critical error (" << e.num() << "): " << e.what() << ", recreating socket" << std::endl;
                socket.close();
                socket = recreate_socket();
                poller = zmq::poller_t<>();
                poller.add(socket, zmq::event_flags::pollin);
            } else {
                std::cout << "Non-transient error (" << e.num() << "): " << e.what() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Unexpected error: " << e.what() << std::endl;
            // Optionally add a small sleep to avoid a busy loop in case of repeated errors
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Clean up
    socket.close();
}

int main() {
    zmq::context_t context(1);
    std::string endpoint = "tcp://*:5555";

    std::thread rep_thread(rep_socket_thread, std::ref(context), endpoint);
    rep_thread.detach();

    // The thread will run indefinitely, handling messages and errors
    // To stop the thread and context cleanly, you should implement a proper shutdown mechanism
    // Example:
    // rep_thread.join();
    // context.shutdown();
    
    std::this_thread::sleep_for(std::chrono::hours(1));  // Keep the main thread running

    return 0;
}
#include <array>
#include <chrono>
#include <cstdio>
#include <format>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <syncstream>
#include <thread>
#include <utility>
#include <vector>

/// The number of producer threads created in this application.
static constexpr int NUMBER_OF_PRODUCERS = 5;

/// The producer thread send a number of messages on a pipe at fixed intervals before terminating.
///
/// @param id 
///     An id number that is added to messages sent by the producer.
/// @param wfd 
///     The file descriptor of the write end of the pipe.
/// @param period 
///     How often to send a message.
/// @param count 
///     The number of messages to send.
static void producerThread(int id, int wfd, std::chrono::milliseconds period, int count) {

    std::osyncstream(std::cout) << "Starting producer " << id << std::endl;
    for (int i = 0; i < count; i++) {
        std::this_thread::sleep_for(period);
        auto message = std::format("Producer {}: {}", id, i);
        write(wfd, message.c_str(), message.size());
    }
    close(wfd);
    std::osyncstream(std::cout) << "Producer " << id << " terminating" << std::endl;;
}

/// Creates a new unnamed pipe.
///
/// @return 
///     The (read_fd, write_fd) of the pipe.
/// @throw std::runtime_error
///     If the call to pipe() fails, this exception is raised.
static std::pair<int, int> createPipe() {

    std::array<int, 2> fds;
    if (pipe(fds.data()) < 0) {
        throw std::runtime_error("Failed to create pipe");
    }
    return std::make_pair(fds[0], fds[1]);
}

/// Creates a producer thread.
///
/// @param id
///     A number to identify the thread. This is placed in messages sent by the producer.
/// @param period
///     The period between sending messages.
/// @param count
///     The number of messages to send before the producer terminates.
/// @return 
///     Returns the read file descriptor to consume data sent by the producer.
static std::pair<int, std::thread> createProducer(int id, std::chrono::milliseconds period, int count) {

    auto [rfd, wfd] = createPipe();
    std::thread t([id, wfd, period, count] { producerThread(id, wfd, period, count); });
    return std::make_pair(rfd, std::move(t));
}

/// Process the the polled file descriptor to determine if the file descriptor has data or has gone into error.
///
/// @param pfd 
///     The polled file descriptor after poll has sucessfully completed.
/// @return
///     True if the pip is closed or in error to indicate that communication has terminated.
/// @throw std::runtime_error
///     If call to read fails.
static bool processPollFd(const struct pollfd &pfd) {

    if (pfd.revents & POLLIN) {
        std::array<char, 128> buf;
        int retval = read(pfd.fd, buf.data(), buf.size());
        if (retval == -1) {
            throw std::runtime_error("Error reading from pipe.");
        }
        std::string message(buf.begin(), buf.begin() + retval);
        std::osyncstream(std::cout) << "Received: " << message << std::endl;
    } else if (pfd.revents & (POLLHUP | POLLERR)) {
        close(pfd.fd);
        return true;
    }
    return false;
}

/// Executes the consumer which performs IO multiplexing using poll and prints received messages.
///
/// @param rfds 
///     The file descriptors for the pipes this consumer should read from.
/// @throw std::runtime_error
///     Throws this error if a system call to read or poll fails.
static void runConsumer(const std::vector<int> &rfds) {

    // Create the array of file descriptrs for the poll system call.
    std::osyncstream(std::cout) << "Starting consumer" << std::endl;
    std::vector<struct pollfd> pollFds;
    for (int rfd : rfds) {
        struct pollfd pdf;
        pdf.events = POLLIN;
        pdf.fd = rfd;
        pollFds.push_back(pdf);
    }

    // Poll for data, process polled file descriptors, and detect when the pipes have closed to terminate.
    size_t terminatedCount = 0;
    while (terminatedCount != pollFds.size()) {
        int retval = poll(pollFds.data(), pollFds.size(), -1);
        if (retval == -1) {
            throw std::runtime_error("Poll failed");
        }
        for (auto pfd : pollFds) {
            if (processPollFd(pfd)) {
                terminatedCount++;
            }
        }
    }
}

/// Launches a set of producer threads, and the main thread will consume messages from them via linux pipes until
/// all the pipes have closed then the application terminates. Messages are printed to console to indicate when 
/// consumers and producers have started and terminated. And the consumer prints received messages. 
int main() {

    // Create producer threads which send messages on pipes.
    std::vector<int> rfds;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUMBER_OF_PRODUCERS; i++) {
        auto [rfd, t] = createProducer(i, std::chrono::milliseconds(i * 50), 3*i + 2);
        threads.push_back(std::move(t));
        rfds.push_back(rfd);
    }

    // Run consuming fuction to receive messages from producers.
    try {
        runConsumer(rfds);
    } catch (const std::exception &e) {
        for (int fd : rfds) {
            close(fd);
        }
    }

    for (auto &t : threads) {
        t.join();
    }

    std::cout << "Application is terminating";
}
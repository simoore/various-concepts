#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <string>

#include "spdlog/spdlog.h"

#include "pub.h"

class PubThread {
public: 

    /// Creates the service object by binding the address and context to this object.
    ///
    /// @param addr
    ///     The ZeroMQ address.
    /// @param context
    ///     The ZeroMQ context for the socket to use.
    PubThread(std::string &&addr, std::shared_ptr<zmq::context_t> context):
        mPubSocket(std::move(addr), context)
    {}

    /// Indicates if the thread is still alive.
    ///
    /// @return 
    ///     True if the thread is alive.
    bool alive() {
        return mThread.joinable();
    }

    /// Starts the thread.
    void start() {
        if (!alive()) {
            mQueue = std::queue<std::optional<std::string>>();
            mThread = std::thread([this](){ service(); });
        }
    }

    /// Stops the thread. Waits for it to join.
    void stop() {
        {
            std::lock_guard lock(mMutex);
            mQueue.push(std::nullopt);
        }
        mMessageSemaphore.release();
        mThread.join();
    }

    /// Queues a message for sending.
    ///
    /// @param message 
    ///     The message to send.
    void send(std::string &&message) {
        std::lock_guard lock(mMutex);
        mQueue.push(std::move(message));
        mMessageSemaphore.release();
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Waits for the producer thread to send a message.
    ///
    /// @return 
    ///     The message on the queue.
    std::optional<std::string> waitForMessage() {
        spdlog::info("Pub thread waiting for message.");
        mMessageSemaphore.acquire();
        std::lock_guard lock(mMutex);
        auto message = std::move(mQueue.front());
        mQueue.pop();
        return message;
    };

    /// This thread works by,
    /// 1. Creating and binding the socket.
    /// 2. Wait for messages. 
    /// 3. If the message is std::nullopt, exit the thread.
    /// 4. Send the message.
    /// 5. If there is an error during send, recreate the socket after a delay.
    void service() {
        bool run = true;
        while (true) {
            mPubSocket.bind();
            while (true) {
                auto messageopt = waitForMessage();
                if (!messageopt) {
                    spdlog::info("Pub thread exit message received.");
                    run = false;
                    break;
                }
                try {
                    spdlog::info("Pub thread sending message.");
                    mPubSocket.send(*messageopt);
                } catch (const zmq::error_t &e) {
                    break;
                }
            }
            if (!run) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } 
        mPubSocket.close();
        spdlog::info("Pub thread is terminating.");
    }

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The publish socket.
    Pub mPubSocket;

    /// The binary semaphore is used to flag when there are messages in the queue.
    std::binary_semaphore mMessageSemaphore{0};

    /// Guards access to the queue.
    std::mutex mMutex;

    /// The message queue for the socket.
    std::queue<std::optional<std::string>> mQueue;

    /// The thread object this service runs in.
    std::thread mThread;

};

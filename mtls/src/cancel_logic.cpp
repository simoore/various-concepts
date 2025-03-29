#include <boost/asio/awaitable.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/system_timer.hpp>

#include <atomic>
#include <stdexcept>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

namespace asio = boost::asio;

static std::atomic<bool> run(true);

/// The exception handle for the coroutine do_task(). We just flag that we have terminated and print the exception
/// message.
///
/// @param e 
///     The caught exception.
static void task_error_handler(std::exception_ptr e) {
    run.store(false);
    try {
        if (e) {
            std::rethrow_exception(e);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Task error: {}", e.what());
    }
}

/// A periodic task that spends most of its time wiating on a timer. This is to test how to cancel task from another
/// task.
static asio::awaitable<void> do_task() {

    auto cs = co_await asio::this_coro::cancellation_state;
    auto executor = co_await asio::this_coro::executor;
    boost::asio::system_timer timer{ co_await boost::asio::this_coro::executor };

    // Allows all cancellation signals to propagate.
    co_await asio::this_coro::reset_cancellation_state(
        asio::enable_total_cancellation()
    );

    while (true) {
        SPDLOG_INFO("Hello");
        timer.expires_after(std::chrono::seconds(1));
        // Using a as_tuple completion handle make the cancellation error return rather than throw an exception.
        co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
        if (cs.cancelled() != asio::cancellation_type::none) {
            SPDLOG_INFO("This task is being cancelled: {}", static_cast<int>(cs.cancelled()));
            break;
        }
    }
    SPDLOG_INFO("Task is exiting");
}

/// In this function we wait for the SIGINT or SIGTERM and we cancel the do_task() coroutine.
///
/// @param signal
///     The cancelation signal for the do_task() coroutine.
asio::awaitable<void> handle_signals(asio::cancellation_signal &signal) {

    auto executor = co_await asio::this_coro::executor;
    auto signal_set = asio::signal_set(executor, SIGINT, SIGTERM);
    auto sig = co_await signal_set.async_wait(asio::use_awaitable);\
    asio::system_timer timer(executor); 

    if (sig == SIGINT) {
        SPDLOG_INFO("Handling SIGINT");
        signal.emit(asio::cancellation_type::total);

        // Poll termination flag. You could create a custom awaitable to poll for you.
        int poll_count = 0;
        bool terminate = false;
        while (true) {
            if (!run.load()) {
                break;
            }
            if (poll_count == 20) {
                terminate = true;
                break;
            }
            timer.expires_after(std::chrono::milliseconds(200));
            co_await timer.async_wait(asio::use_awaitable);
            poll_count++;
        }

        if (terminate) {
            SPDLOG_INFO("Emitting termination signal");
            signal.emit(asio::cancellation_type::terminal);
        }

        SPDLOG_INFO("Finished handling SIGINT.");
    } else {
        SPDLOG_INFO("Handling SIGTERM.");
        signal.emit(asio::cancellation_type::terminal);
    }
}

/// Run the I/O service on the requested number of threads. If we request N threads, we create N - 1 new threads 
/// because we use main thread as well. This function returns when all tasks have completed.
///
/// @param ioc
///     The boost::asio io_context.
/// @param threads
///     The number of threads available to the io_context.
static void run_service(asio::io_context &ioc, int threads) {
    SPDLOG_INFO("IO Context is running.");

    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (int i = 0; i < threads - 1; ++i) {
        v.emplace_back([&ioc] { ioc.run(); });
    }
    ioc.run();
    SPDLOG_INFO("IO Context has exited.");

    // When the threads terminate due to a signal, we capture them here.
    for (auto &t : v) {
        t.join();
    }
    SPDLOG_INFO("Child threads have exited.");
}

/*****************************************************************************/
/********** MAIN FUNCTION ****************************************************/
/*****************************************************************************/

int main() {
    const int threads = 3;

    // The io_context is required for all I/O using boost::asio.
    asio::io_context ioc{threads};

    // This is the signal used to communicate termination.
    asio::cancellation_signal cancel_listen;

    // Spawn the task async task.
    auto task_awaitable = do_task();
    auto task_token = asio::bind_cancellation_slot(cancel_listen.slot(), task_error_handler);
    asio::co_spawn(ioc, std::move(task_awaitable), task_token);

    // It is detached because we expect no return values.
    asio::co_spawn(ioc, handle_signals(cancel_listen), asio::detached);

    // Run until all tasks are complete.
    run_service(ioc, threads);

    return 0;
}

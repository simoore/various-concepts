#include <coroutine>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FIRST COROUTINE
// ---------------
// This explains some of the execution flow of a simple coroutine 
// https://itnext.io/c-20-coroutines-complete-guide-7c3fc08db89d
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace firstcoroutine {

// The caller-level type
struct Task {

    // The coroutine level type
    struct promise_type {
        using Handle = std::coroutine_handle<promise_type>;

        Task get_return_object() { 
            std::cout << "2) Creating the coroutine handler object is part of the initialization" << std::endl;
            return Task{Handle::from_promise(*this)}; 
        }

        std::suspend_always initial_suspend() { 
            std::cout << "3) When the coroutine is ready, this is called first" << std::endl;
            std::cout << "   std::suspend_always initializes the coroutine in a suspended state" << std::endl;
            return {}; 
        }

        std::suspend_never final_suspend() noexcept { 
            std::cout << "8) We have hit the final co_* statement in the coroutine" << std::endl;
            return {}; 
        }

        void return_void() {
            std::cout << "7) I thinks co_return leads us to come here..." << std::endl;
        }
        void unhandled_exception() {}
    };

    explicit Task(promise_type::Handle coro) : mCoro(coro) {}

    // Delete the copy construction to make Task move-only
    Task(const Task&) = delete;

    // Delete the copy assignment operator to make Task move-only
    Task& operator=(const Task&) = delete;

    // Define the move assignment operator
    Task& operator=(Task&& t) noexcept {
        if (this == &t) {
            return *this;
        }
        if (mCoro) { 
            mCoro.destroy();
        }
        mCoro = t.mCoro;
        t.mCoro = {};
        return *this;
    }

    void destroy() { 
        mCoro.destroy(); 
    }

    void resume() { 
        std::cout << "5) The caller function calls resume" << std::endl;
        mCoro.resume(); 
        std::cout << "9) The coroutine returns from its execution after suspending itself" << std::endl;
    }

private:

    promise_type::Handle mCoro;
};

Task myCoroutine() {
    // This function is the coroutine. When we call myCoroutine, its state is allocated on the heap. When we get to 
    // this co_return call, the initial_suspend() function is called and since the return object is 
    // std::suspend_always, we return execution to the caller
    std::cout << "6) The coroutine resumes from its initial suspension" << std::endl;
    co_return; 
    std::cout << "*** We should never get here in the current application" << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- FIRST COROUTINE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    // This object is the return value of Task::promise_type::get_return_object(), it returns out task object
    // which contains the handle to the std::coroutine which was create using a promise object that is automatically
    // created for us somewhere.
    std::cout << "1) This is the start of the application" << std::endl;
    auto c = myCoroutine();
    std::cout << "4) Since the coroutine starts suspended, we return to the caller function" << std::endl;
    c.resume();
    std::cout << "10) We exit and everything gets destroyed and cleaned-up" << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERATOR EXAMPLE
// -----------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace generatorexample {

struct Generator {

    struct promise_type {

        using Handle = std::coroutine_handle<promise_type>;        
        
        Generator get_return_object() {
            return Generator{Handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { 
            return {}; 
        }

        std::suspend_always final_suspend() noexcept { 
            std::cout << "*** these is no final suspend in this example" << std::endl;
            return {}; 
        }

        std::suspend_always yield_value(int value) {
            currentValue = value;
            return {};
        }

        void unhandled_exception() {}

        int currentValue;
    };    
    
    explicit Generator(promise_type::Handle coro) : mCoro(coro) {}

    ~Generator() {
        if (mCoro) 
            mCoro.destroy();
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& t) noexcept : mCoro(t.mCoro) { 
        t.mCoro = {};
    }

    Generator& operator=(Generator&& t) noexcept {
        if (this == &t) 
            return *this;
        if (mCoro) 
            mCoro.destroy();
        mCoro = t.mCoro;
        t.mCoro = {};
        return *this;
    }    
    
    int getNext() {
        mCoro.resume();
        // When the coroutine suspends we can read the latest yielded return value
        return mCoro.promise().currentValue;
    }
    
private:
    promise_type::Handle mCoro;
};

Generator myCoroutine() {
    int x = 0;
    while (true) {
        co_yield x++;
    }
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- GENERATOR EXAMPLE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    auto c = myCoroutine();
    int x = 0;
    while ((x = c.getNext()) < 10) {
        std::cout << x << "\n";
    }
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AWAITABLES
// ----------
// Fundamentally co_return and co_yield are forms of co_await that are handle in a certain way by the promise object.
// Calls to co_await execute 'await_transform' function in the promise and they must return an Awaitable object
// that tells the coroutine what to do.
//
// We can create our own awaitables that conform to some interface.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace awaitables {

struct Promise;
struct Coroutine : std::coroutine_handle<Promise> {
    using promise_type = Promise;
};

struct Promise {
    Coroutine get_return_object() {
        std::cout << "2) Coroutine initialization" << std::endl;
        return {Coroutine::from_promise(*this)}; 
    }

    std::suspend_never initial_suspend() noexcept { 
        std::cout << "3) Start coroutine execution after initialization" << std::endl;
        return {}; 
    }
    std::suspend_never final_suspend() noexcept { 
        std::cout << "11) co_return called or we readched end of execution" << std::endl;
        return {}; 
    }

    // Cannot define return_void and return_value in the same promise_type
    // void return_void() {
    //     std::cout << "10) co_return called or we readched end of execution" << std::endl;
    // }

    void return_value(int i) {
        std::cout << "10) co_return <int> was called" << std::endl;
    }

    void unhandled_exception() {
        std::cout << "*** unhandled_exception" << std::endl;
    }

    // Called by the 'c_await expr' statement in the coroutine (decltype(expr) == std::suspend_always)
    std::suspend_always await_transform(std::suspend_always s) {
        std::cout << "   'co_await std::suspend_always' was called." << std::endl;
        return s;
    }

    // Called by the 'c_await expr' statement in the coroutine (decltype(expr) == std::suspend_never)
    std::suspend_never await_transform(std::suspend_never s) {
        std::cout << "   'co_await std::suspend_never' was called." << std::endl;
        return s;
    }
};

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- AWAITABLES" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::cout << "1) Start of caller" << std::endl;
    
    Coroutine handle = [](void) -> Coroutine {
        std::cout << "4) Start immediately" << std::endl;
        co_await std::suspend_always{};
        std::cout << "6) We have resumed" << std::endl;
        co_await std::suspend_never{};
        std::cout << "7) We never stopped resumed" << std::endl;
        co_await std::suspend_always{};
        std::cout << "9) We have resumed again" << std::endl;
        co_return 15;
        std::cout << "** We will never get here" << std::endl;
        // If we go off the end it is equivelent to calling co_return with a void return type.
    }();

    std::cout << "5) Back to caller after first co_await called" << std::endl;
    handle.resume();
    std::cout << "8) Back to caller after second co_await called" << std::endl;
    handle.resume();
    std::cout << "12) Back to caller hitting end of coroutine (equivalent to co_return)" << std::endl;
    std::cout << "    This the coroutine done: " << (handle.done() ? "true" : "false") << std::endl;
    // Calling resume() again on the terminated coroutine is considered undefined behehaviour
    // handle.resume();
    std::cout << "13) end of caller" << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    firstcoroutine::run();
    generatorexample::run();
    awaitables::run();
}
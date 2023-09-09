#include <atomic>
#include <functional>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include "thread_guard.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Threads
// -------
// Here we launch a thread using the function `func`. We wait for it to terminate using `join()`. We can use any 
// callable objects such as lambdas and functors. The way the `join()` functions are called can cause the print
// statements to be mixed up.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace threads {

void func() {
    std::cout << std::this_thread::get_id() << " Hello from func" << std::endl;
}

class CallableClass {
public:
    void operator()() {
        std::cout << std::this_thread::get_id() << " Hello from functor" << std::endl;
    }
};

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- THREADS" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func);
    
    CallableClass obj;
    std::thread thread2(obj);

    std::thread thread3([]() {
        std::cout << std::this_thread::get_id() << " Hello from lambda" << std::endl;
    });

    thread1.join();
    thread2.join();
    thread3.join();

    std::cout << std::this_thread::get_id() << " Hello from main" << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Joinability
// -----------
// You must call join or detatch on a thread before the thread object is destructed. If a thread is destructed without 
// a join an exception is thrown. You can only call join or detatch once in a threads lifetime. Afterwards a thread
// becomes unjoinable. A thread constructed without a callable object are not joinable.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace joinability {

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- JOINABILITY" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1([]() {
        std::cout << std::this_thread::get_id() << " Hello from lambda" << std::endl;
    });

    if (thread1.joinable()) {
        std::cout << "This thread is joinable" << std::endl;
    } else {
        std::cout << "This thread is not joinable" << std::endl;
    }

    thread1.join();

    if (thread1.joinable()) {
        std::cout << "This thread is joinable" << std::endl;
    } else {
        std::cout << "This thread is not joinable" << std::endl;
    }

    std::cout << std::this_thread::get_id() << " Hello from main" << std::endl;

    std::thread thread2;

    if (thread2.joinable()) {
        std::cout << "This thread is joinable" << std::endl;
    } else {
        std::cout << "This thread is not joinable" << std::endl;
    }
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Join and Detach
// ---------------
// One blocks one doesn't
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace joinanddetach {

void func1() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "Hello from func1" << std::endl;
}

void func2() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "Hello from func2" << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- JOIN AND DETACH" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func1);
    std::thread thread2(func2);
    thread1.detach();
    std::cout << "This is after the detach" << std::endl;
    thread2.join();
    std::cout << "This is after the join" << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exceptions with join
// --------------------
// Use RAII to join a thread so the thread handler is not deleted before the thread has finished.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace exceptionswithjoin {

void threadFunc() {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "Hello from threadFunc" << std::endl;
}

void exceptFunc() {
    std::cout << "Hello from exceptFunc" << std::endl;
    throw std::runtime_error("this is a runtime error");
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- EXCEPTIONS WITH JOIN" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(threadFunc);
    ThreadGuard guard(thread1);
    try
    {
        exceptFunc();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parameters to Threads
// ---------------------
// Use std::ref if you want to pass by reference. Be mindful of the lifetime of any variables passed by reference.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace parameterstothreads {

void func1(int x, int y) {
    std::cout << "X + Y = " << (x + y) << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PARAMETERS TO THREADS" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func1, 8, 9);
    thread1.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transferring Ownership
// ----------------------
// Use the move assignment. Cannot move if a thread handler is being used.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace transferringownership {

void func1(void) {
    std::cout << "Hello from func1" << std::endl;
}

void func2(void) {
    std::cout << "Hello from func2" << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- TRANSFERRING OWNERSHIP" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func1);

    // We have moved the ownship of the func1 thread to thread2 and invalidated thread1.
    std::thread thread2 = std::move(thread1);

    // We can now use thread1 for another thread. This is a move because the RHS is an rvalue.
    thread1 = std::thread(func2);

    thread1.join();
    thread2.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful Functions
// ----------------
// sleep_for()      Puts the thread to sleep for at least the specified amount of time
// yield()          Gives up the remainder of its time slice and puts itself back in the scheduler queue 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace usefulfunctions {

void func1(void) {
    std::cout << "Hello from thread id : " << std::this_thread::get_id() << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- USEFUL FUNCTIONS" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func1);
    std::thread thread2(func1);
    std::thread thread3(func1);
    std::thread thread4; // Not an active thread, thread id 0

    std::cout << "Thread1 id : " << thread1.get_id() << std::endl;
    std::cout << "Thread2 id : " << thread2.get_id() << std::endl;
    std::cout << "Thread3 id : " << thread3.get_id() << std::endl;
    std::cout << "Thread4 id : " << thread4.get_id() << std::endl;

    thread1.join();
    thread2.join();
    thread3.join();

    // Terminated threads have thread id 0 - it seems that 
    std::cout << "Thread3 id : " << thread3.get_id() << std::endl;

    // Determines the maximum number of simultaneous threads that can run on a given machine.
    int allowedThreads = std::thread::hardware_concurrency();
    std::cout << "Allowed thread count in my device : " << allowedThreads << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parallel Accumulate
// -------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace parallelaccumulate {

static constexpr int sMinBlockSize = 1000;

void sequentialAccumulateTest(void) {

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int sum = std::accumulate(v.begin(), v.end(), int(0));
    std::cout << "Sequential sum: " << sum << std::endl;

    int product = std::accumulate(v.begin(), v.end(), int(0), std::multiplies<int>());
    std::cout << "Sequential product: " << product << std::endl;

    auto dashFold = [](std::string a, int b) {
        return std::move(a) + "-" + std::to_string(b);
    };
    std::string s = std::accumulate(std::next(v.begin()), v.end(), std::to_string(v[0]), dashFold);
    std::cout << "Sequential dash fold: " << s << std::endl;
}

template <typename iterator, typename T>
void accumulate(iterator start, iterator end, T &ref) {
    ref = std::accumulate(start, end, T(0));
}

template <typename iterator, typename T>
T parallelAccumulate(iterator start, iterator end, T &ref) {
    
    int inputSize = std::distance(start, end);
    int allowedThreadsByElements = inputSize / sMinBlockSize;
    int allowedThreadsByHardware = std::thread::hardware_concurrency();
    int numThreads = std::min(allowedThreadsByElements, allowedThreadsByHardware);
    int blockSize = (inputSize + 1) / numThreads;

    std::vector<T> results(numThreads);
    std::vector<std::thread> threads(numThreads - 1);

    iterator last;
    for (int i = 0; i < numThreads - 1; i++) {
        last = start;
        std::advance(last, blockSize);
        threads[i] = std::thread(accumulate<iterator, T>, start, last, std::ref(results[i]));
        start = last;
    }

    results[numThreads - 1] = std::accumulate(start, end, T(0));
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return std::accumulate(results.begin(), results.end(), ref);
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PARALLEL ACCUMULATE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    sequentialAccumulateTest();

    const int size = 8000;
    int *myArray = new int[size];
    int ref = 0;

    srand(0);

    for (size_t i = 0; i < size; i++) {
        myArray[i] = rand() % 10;
    }

    int retVal = parallelAccumulate<int *, int>(myArray, myArray + size, ref);
    std::cout << "Accumulated value : " << retVal << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread Local
// ----------------
// If `i` wasn't thread_local, the threads would print 123 rather than 111
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace threadlocal {

thread_local std::atomic<int> i = 0;

void func1(void) {
    ++i;
    std::cout << "i is : " << i.load() << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- THREAD LOCAL" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(func1);
    std::thread thread2(func1);
    std::thread thread3(func1);

    thread1.join();
    thread2.join();
    thread3.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    threads::run();
    joinability::run();
    joinanddetach::run();
    exceptionswithjoin::run();
    parameterstothreads::run();
    transferringownership::run();
    usefulfunctions::run();
    parallelaccumulate::run();
    threadlocal::run();
}
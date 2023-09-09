#include <condition_variable>
#include <cmath>
#include <cstdio>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <thread>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Condition Variable
// ------------------
// Use to notify a sleeping thread when a condition is met. Similar to a semaphore.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace conditionvariable {

int distanceCovered = 0;
int totalDistance = 5;
std::mutex mut;
std::condition_variable cv;

// This function is like a producer generating data until some condition is met and it can wake up a thread waiting 
// on that condition.
void keepMoving(void) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        distanceCovered++;
        if (distanceCovered == totalDistance) {
            cv.notify_one();
        }
    }
}

void waitingForRightDistance(void) {
    std::unique_lock<std::mutex> ul(mut);
    cv.wait(ul, [] { return distanceCovered == totalDistance; });
    std::cout << "I have waited the right distance." << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- CONDITION VARIABLE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(waitingForRightDistance);
    std::thread thread2(keepMoving);
    thread1.join();
    thread2.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread Safe Queue
// -----------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace threadsafequeue {

template <typename T>
class ThreadSafeQueue {
    std::mutex m;
    std::condition_variable cv;
    std::queue<std::shared_ptr<T>> queue;

public:

    void push(T value) {
        std::lock_guard<std::mutex> lg(m);
        queue.push(std::make_shared<T>(value));
        cv.notify_one();
    }

    std::shared_ptr<T> pop(void) {
        std::lock_guard<std::mutex> lg(m);
        if (queue.empty()) {
            return std::shared_ptr<T>();
        } else {
            std::shared_ptr<T> ref(queue.front());
            queue.pop();
            return ref;
        }
    }

    std::shared_ptr<T> waitPop(void) {
        std::unique_lock<std::mutex> lg(m);
        cv.wait(lg, [this]{ return !queue.empty(); });
        std::shared_ptr<T> ref = queue.front();
        queue.pop();
        return ref;
    }

    bool empty(void) {
        std::lock_guard<std::mutex> lg(m);
        return queue.empty();
    }

};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Futures
// -------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace futures {

int howOldIsTheUniverse() {
    return 5000;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- FUTURES" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::future<int> answerFuture = std::async(howOldIsTheUniverse);
    std::cout << "Do other calculations" << std::endl;
    std::cout << "The answer is " << answerFuture.get() << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Async Task
// ----------
// Futures can either be launched in a new thread, executed later in the same thread, or you can let the program
// decided.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace asynctask {

void printing() {
    std::cout << "printing runs on-" << std::this_thread::get_id() << std::endl;
}

int addition(int x, int y) {
    std::cout << "addition runs on-" << std::this_thread::get_id() << std::endl;
    return x + y;
}

int subtraction(int x, int y) {
    std::cout << "subtraction runs on-" << std::this_thread::get_id() << std::endl;
    return x - y;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- ASYNC TASK" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::cout << "main thread id -" << std::this_thread::get_id() << std::endl;

    int x = 100;
    int y = 50;

    std::future<void> f1 = std::async(std::launch::async, printing);
    std::future<int> f2 = std::async(std::launch::deferred, addition, x, y);
    std::future<int> f3 = std::async(std::launch::deferred | std::launch::async, subtraction, x, y);

    f1.get();
    std::cout << "value recieved using f2 future -" << f2.get() << std::endl;
    std::cout << "value recieved using f3 future -" << f3.get() << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Accumulate Algorithm
// --------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace accumulate {

int MIN_ELEMENT_COUNT = 1000;

template <typename iterator>
int parallelAccumulate(iterator begin, iterator end) {

    long length = std::distance(begin, end);
    if (length <= MIN_ELEMENT_COUNT) {
        std::cout << std::this_thread::get_id() << std::endl;
        return std::accumulate(begin, end, 0);
    }

    iterator mid = begin;
    std::advance(mid, (length + 1) / 2);

    auto f1 = std::async(std::launch::deferred | std::launch::async, parallelAccumulate<iterator>, mid, end);
    auto sum = parallelAccumulate(begin, mid);
    return sum + f1.get(); 
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- ACCUMULATE ALGORITHM" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::vector<int> v(10000, 1);
    std::cout << "The sum is " << parallelAccumulate(v.begin(), v.end()) << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Packaged Tasks
// --------------
// Can pass the packaged task around like std::function and called at a later time.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace packagedtasks {

int add(int x, int y) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "add function runs in : " << std::this_thread::get_id() << std::endl;
    return x + y;
}

void taskThread(void) {
    std::packaged_task<int(int, int)> task1(add);
    std::future<int> future1 = task1.get_future();
    std::thread thread1(std::move(task1), 5, 6);
    thread1.detach();
    std::cout << "task thread- " << future1.get() << std::endl;
}

void taskNormal(void) {
    std::packaged_task<int(int, int)> task1(add);
    std::future<int> future1 = task1.get_future();
    task1(7, 8);
    std::cout << "task normal - " << future1.get() << std::endl;
}

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PACKAGED TASKS" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    taskNormal();
    taskThread();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Promises
// --------
// Allow inter-thread communication.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace promises {

void printInt(std::future<int> &fut) {
    printf("waiting for value from print thread \n");
    printf("value: %d\n", fut.get());   // Waits for the promise to set the value and is gotten here.
}

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PROMISES" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread printThread(printInt, std::ref(fut));

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    printf("setting the value int he main thread \n");
    prom.set_value(10);
    printThread.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Promises with Exceptions
// ------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace promiseswithexceptions {

void throwException(void) {
    throw std::runtime_error("input cannot be negative");
}

void calculateSquareRoot(std::promise<int> &prom) {
    // We are going to calculate the result and pass it to the print result thread
    // The square root function throws and exception for negative numbers
    int x = 1;
    std::cout << "Please, enter an integer value ";
    try {
        std::cin >> x;
        if (x < 0) {
            throwException();
        }
        prom.set_value(std::sqrt(x));
    } catch (std::exception &) {
        prom.set_exception(std::current_exception());
    }
}

void printResult(std::future<int> &fut) {
    // We are going to print the result generated in the calculation thread.
    try {
        int x = fut.get();
        printf("value: %d \n", x);
    } catch (std:: exception &e) {
        printf("[Exception caught: %s]\n", e.what());
    }
}

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PROMISES WITH EXCEPTIONS" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread printThread(printResult, std::ref(fut));
    std::thread calculationThread(calculateSquareRoot, std::ref(prom));

    printThread.join();
    calculationThread.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shared Futures
// --------------
// Futures are invalid after the get call, cannot call it twice.
// There is the future.valid() function, but be careful of race-conditions
// Use shared_future
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sharedfutures {

void printResult(std::shared_future<int> &fut) {
    printf("The value is: %d\n", fut.get());
}

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- SHARED FUTURES" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::promise<int> prom;
    std::shared_future<int> fut = prom.get_future();

    std::thread thread1(printResult, std::ref(fut));
    std::thread thread2(printResult, std::ref(fut));

    prom.set_value(5);

    thread1.join();
    thread2.join();
}

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    // conditionvariable::run(); this one has an infinite loop
    futures::run();
    asynctask::run();
    accumulate::run();
    packagedtasks::run();
    promises::run();
    // promiseswithexceptions::run();  // asks for user input
    sharedfutures::run();
}

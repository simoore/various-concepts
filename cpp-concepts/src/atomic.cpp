#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPARE AND EXCHANGE
// --------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace compareandexchange {

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- COMPARE AND EXCHANGE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::atomic<int> x(20);

    int expectedValue = 20;
    std::cout << "previous expected value: " << expectedValue << std::endl;

    // Since the atomic already contains 20, then the return val should be true and the atomic is updated to the 
    // value of 6. If the expected value doesn't equal the stored value in the atomic, it is updated to the 
    // expected value. This operation can fail, and the return value indicates if this is so.
    bool returnVal = x.compare_exchange_weak(expectedValue, 6);

    std::cout << "operation sucessful     :" << (returnVal ? "yes" : "no") << std::endl;
    std::cout << "current expectedValue   :" << expectedValue << std::endl;
    std::cout << "current x               :" << x.load() << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HAPPEN BEFORE
// -------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace happenbefore {

std::atomic<bool> dataReady = false;
std::vector<int> dataVector;

// Waits til data is ready and prints the result.
void readerFunc() {
    while (!dataReady) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    // The push_back happened before reading this value due to how the atomic<bool> is used.
    std::cout << dataVector[0] << std::endl;
}

void writerFunc() {
    dataVector.push_back(3);
    dataReady.store(true);
}

void run(void) {
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- HAPPEN BEFORE" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::jthread thread1(readerFunc);
    std::jthread thread2(writerFunc);
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY ORDERING SEQUENTIAL CONSISTENT
// -------------------------------------
// Consider memory ordering as changing the view a thread has of an atomic variable. 
// Every thread see the global view of the atomic variable when using sequential consistent memory ordering.
// Therefore at least one of the read threads is guarenteed to increment the atomic z in this example.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace memoryorderingsequentialconsistent {

std::atomic<bool> x, y;
std::atomic<int> z;

void writeX(void) {
    x.store(true, std::memory_order_seq_cst);
}

void writeY(void) {
    y.store(true, std::memory_order_seq_cst);
}

void readXThenY(void) {
    while (!x.load(std::memory_order_seq_cst));
    if (y.load(std::memory_order_seq_cst)) {
        z++;
    }
}

void readYThenX(void) {
    while (!y.load(std::memory_order_seq_cst));
    if (x.load(std::memory_order_seq_cst)) {
        z++;
    }
}

void run(void) {
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- MEMORY ORDERING SEQUENTIAL CONSISTENT" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    x = false;
    y = false;
    z = 0;

    std::jthread thread1(writeX);
    std::jthread thread2(writeY);
    std::jthread thread3(readXThenY);
    std::jthread thread4(readYThenX);

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    std::cout << "z is: " << z << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY ORDERING RELAXED
// -----------------------
// View of the atomic variables doesn't need to be consistent between threads.
// Since the writer thread is using memory order relaxed, the compiler is under no obligation to ensure that x is 
// written written to before y since they have no dependance. All the atomic guarentees is that threads won't see
// a corrupt state of the variable. For the reader thread, it doesn't have to have a consistent view of the atomic
// variable in the writer thread since is using memory_order_relaxed. In this case the atomic isn't used for 
// synchronization, it just protects from an corrupt read.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace memoryorderingrelaxed {

std::atomic<bool> x, y;
std::atomic<int> z;

void writeXThenY(void) {
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void readYThenX(void) {
    while (!y.load(std::memory_order_relaxed));
    if (x.load(std::memory_order_relaxed)) {
        z++;
    }
}

void run(void) {
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- MEMORY ORDERING RELAXED" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    x = false;
    y = false;
    z = 0;

    std::jthread thread1(writeXThenY);
    std::jthread thread2(readYThenX);

    thread1.join();
    thread2.join();

    std::cout << "z is: " << z << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SYNC WITH
// ---------
// An atomic store and atomic load are synchronized if the store is using memory_order_release and the read is using
// memory_order_aquire. In this case, the x.store that happened before y.store will be synchronized with the 
// y.load, thus z will always be incremented.
//
// TRANSITIVE SYNCHRONIZATION
// --------------------------
// Say there are three threads A, B, and C. If there is a sync with relationship between A and B, and a sync with
// relationship between B and C, then there is a sync with relationship between A and C
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace syncwith {

std::atomic<bool> x, y;
std::atomic<int> z;

void writeXThenY(void) {
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_release);
}

void readYThenX(void) {
    while (!y.load(std::memory_order_acquire));
    if (x.load(std::memory_order_relaxed)) {
        z++;
    }
}

void run(void) {
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- SYNC WITH" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    x = false;
    y = false;
    z = 0;

    std::jthread thread1(writeXThenY);
    std::jthread thread2(readYThenX);

    thread1.join();
    thread2.join();

    std::cout << "z is: " << z << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MEMORY ORDER CONSUME
// --------------------
// Carries-a-dependency-to: if the result of an operation A is used by an operation B, then A 
//      carries-a-dependency-to B.
// Dependency-order-before: if an operation in another thread depends on and is synchronized to a result of an 
//  operation. In the current thread.
//
// memory_order_consume only applies memory reording restrictions to instruction that have a direct dependency on
// the value being synchronized. memory_order_acquire ensures that all instructions proceeding it in a thread 
// a seen to other threads.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace memoryorderconsume {

std::atomic<bool> x, y;
std::atomic<int> z;

void writeXThenY(void) {
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_release);
}

void readYThenX(void) {
    // With memory order relaxed, we ensure the y.store and the y.load a synchronized (they have a consistent view),
    // but it no longer prevents memory reordering in this fuction. That is the x.load can be moved since it doesn't
    // have a dependancy on y. Therefore we cannot guarentee that z will be incremented.
    while (!y.load(std::memory_order_consume));
    if (x.load(std::memory_order_relaxed)) {
        z++;
    }
}

void run(void) {
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- MEMORY ORDER CONSUME" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    x = false;
    y = false;
    z = 0;

    std::jthread thread1(writeXThenY);
    std::jthread thread2(readYThenX);

    thread1.join();
    thread2.join();

    std::cout << "z is: " << z << std::endl;
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    compareandexchange::run();
    happenbefore::run();
    memoryorderingsequentialconsistent::run();
    memoryorderingrelaxed::run();
    syncwith::run();
    memoryorderconsume::run();
}
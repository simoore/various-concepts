#include <iostream>
#include <list>
#include <mutex>
#include <memory>
#include <stack>
#include <thread>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mutexes
// -------
// Lock guards call unlock() for us in their destructor.
// Some notes on locks: https://stackoverflow.com/questions/43019598/stdlock-guard-or-stdscoped-lock
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace mutexes {

std::list<int> myList;
std::mutex mut;

void addToList(const int &x) {
    std::lock_guard<std::mutex> lg(mut);
    myList.push_front(x);
}

void size() {
    mut.lock();
    int size = myList.size();
    mut.unlock();
    std::cout << "size of the list is: " << size << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- MUTEXES" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(addToList, 4);
    std::thread thread2(addToList, 11);
    thread1.join();
    thread2.join();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thread Safe Stack
// -----------------
// To avoid inherit by interface race conditions, you simply can't wrap the existing stack API in mutexes. Some
// functions must be called together in the one critical section such as checking the if the stack is empty 
// before trying to pop from it.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace threadsafestack {

template <typename T>
class Stack {
    std::stack<std::shared_ptr<T>> mStack;
    std::mutex mMutex;

public:

    void push(T element) {
        std::lock_guard<std::mutex> lg(mMutex);
        mStack.push(std::make_shared<T>(element));
    }

    std::shared_ptr<T> pop(void) {
        std::lock_guard<std::mutex> lg(mMutex);
        if (mStack.empty()) {
            throw std::runtime_error("stack is empty");
        }
        std::shared_ptr<T> res(mStack.top());
        mStack.pop();
        return res;
    }

    void pop(T &value) {
        std::lock_guard<std::mutex> lg(mMutex);
        if (mStack.empty()) {
            throw std::runtime_error("stack is empty");
        }
        value *(mStack.top().get());
        mStack.pop();
    }

    bool empty(void) {
        std::lock_guard<std::mutex> lg(mMutex);
        return mStack.empty();
    }

    size_t size(void) {
        std::lock_guard<std::mutex> lg(mMutex);
        return mStack.size();
    }
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unique locks
// ------------
// Doesn't have to aquire lock on construction.
// They can be moved.
// Still unlocks on desstruction
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace uniquelock {

std::mutex mMutex1;
std::mutex mMutex2;

void transfer(void) {
    std::cout << std::this_thread::get_id() << " hold the lock from both mutexes" << std::endl;

    // We can defer the lock of a unique lock
    std::unique_lock<std::mutex> ul1(mMutex1, std::defer_lock);
    std::unique_lock<std::mutex> ul2(mMutex2, std::defer_lock);
    std::lock(ul1, ul2);

    std::cout << std::this_thread::get_id() << " transferring" << std::endl;
}

std::unique_lock<std::mutex> func1(void) {
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    std::cout << "func1() Performing operation under lock" << std::endl;
    return lock;
}

void func2() {
    std::unique_lock<std::mutex> lock(func1());
    std::cout << "func2() Performing operation under same lock as func1()" << std::endl;
}

void run() {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- UNIQUE LOCK" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::thread thread1(transfer);
    std::thread thread2(transfer);
    thread1.join();
    thread2.join();

    // This example shows how unique_locks can be moved.
    func2();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    mutexes::run();
    uniquelock::run();
}
#include <barrier>
#include <cstdio>
#include <thread>
#include <latch>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#ifdef USE_BOOST
#include "boost/asio.hpp"   // thread pools
#endif
#include <future>

///////////////////////////////////////////////////////////////////////////////
// Using a condition variable. This implements a condition that makes a thread 
// wait until a certain condition is met before it accesses a shared resource.
//
// A condition variable + mutex combine to creates a construct called a monitor.
//
// There are three opertors on a condition variable:
// - wait           
//  When an thread aquires the mutex, but the condition isn't met. The thread calls
//  the wait operation to add itself to a queue to wait until the condition is met.
// - signal
//  Notify one object in the queue so it can attempt to aquire the lock.
// - broadcast
//  Wakes up all threads in the queue.
//
// Can use a condition variable to signal to threads when the state of shared 
// queue has changed.
//
// Note that the condition variable is not the condition, but a queue to store
// waiting threads.
///////////////////////////////////////////////////////////////////////////////

namespace ConditionVariableBusyWaitingDemo {
int inventory = 10;         // number of items left in our imagined inventory.
std::mutex inventoryLock;   // only one thread can take from the inventory at a time.

// Busy waiting is when a thread keeps coming back, takes the lock, and then checks the
// condition over and over, consuming CPU cycles.
void takeInventoryBusyWait(int id) {
    int failedToTakeInventoryCount = 0;
    while (inventory > 0) {
        // Unqiue locks take ownership of a mutex and will be soley responsible for locking
        // and unlocking. It locks on construction and unlocks on destruction, and has
        // functions to change the state of the lock.
        std::unique_lock<std::mutex> uniqueLock(inventoryLock);
        // The % 2 statement makes the threads take turns at taking items from inventory.
        if ((id == inventory % 2) && (inventory > 0)) { 
            inventory--;
        } else {
            failedToTakeInventoryCount++; // count failed accesses to shared resource.
        }
    }
    printf("Thread %d busy waited with %u failed attempts to take inventory.\n", id, failedToTakeInventoryCount);
}

void conditionVariableDemo() {
    std::thread threads[2];
    for (int i = 0; i < 2; i++) {
        threads[i] = std::thread(takeInventoryBusyWait, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}
} // namespace ConditionVariableDemo

namespace ConditionVariableDemo {
int inventory = 10;         // number of items left in our imagined inventory.
std::mutex inventoryLock;   // only one thread can take from the inventory at a time.
std::condition_variable inventoryTaken;

// Busy waiting is when a thread keeps coming back, takes the lock, and then checks the
// condition over and over, consuming CPU cycles.
void takeInventoryBusyWait(int id) {
    int failedToTakeInventoryCount = 0;
    while (inventory > 0) {
        // Use a unique lock to enable locking & unlocking function, and to unlock 
        // variable when exiting scope.
        std::unique_lock<std::mutex> uniqueLock(inventoryLock);
        while ((id == inventory % 5) && (inventory > 0)) { 
            failedToTakeInventoryCount++; // count failed accesses to shared resource.
            inventoryTaken.wait(uniqueLock);
        } 
        if (inventory > 0) {
            inventory--;
            uniqueLock.unlock();
            // careful, sometimes you need to notify all to check the condition
            // less one of the threads terminates, or waits and doesn't notify any
            // other thread.
            inventoryTaken.notify_all();
        }
    }
    printf("Thread %d busy waited with %u failed attempts to take inventory.\n", id, failedToTakeInventoryCount);
}

void conditionVariableDemo() {
    std::thread threads[5];
    for (int i = 0; i < 5; i++) {
        threads[i] = std::thread(takeInventoryBusyWait, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}
} // namespace ConditionVariableDemo

///////////////////////////////////////////////////////////////////////////////
// Producer-Consumer
// * Queues - FIFO, there are a number synchronization issues
// * If producer is faster than the consumer, you can get buffer overflow.
// * Pipeline architectures1
///////////////////////////////////////////////////////////////////////////////

namespace PipelineDemo {
class Pipeline {
    public:
        void addToPipeline(int element) {
            std::unique_lock<std::mutex> lock(mMutex);
            mQueue.push(element);
            lock.unlock();
            mConditionVar.notify_one();
        }

        int takeFromPipeline() {
            std::unique_lock<std::mutex> lock(mMutex);
            while (mQueue.empty()) {
                mConditionVar.wait(lock);
            }
            int element = mQueue.front();
            mQueue.pop();
            return element;
        }

    private:
        std::queue<int> mQueue;
        std::mutex mMutex;
        std::condition_variable mConditionVar;
};

Pipeline pipeline = Pipeline();

void producer() {
    for (int i = 0; i < 1000000; i++) {
        pipeline.addToPipeline(1);
    }
    pipeline.addToPipeline(-1); // indicate no more elements
    printf("Producer is done\n");
}

void consumer() {
    int elementsTaken = 0;
    while (true) {
        int element = pipeline.takeFromPipeline();
        if (element == -1) { // check for last element
            printf("Consumer took %d elements.\n", elementsTaken);
            pipeline.addToPipeline(-1); // make sure other threads see end of queue
            return;
        } else {
            elementsTaken += element; // consume the element
        }
    }
}

void pipelineExample() {
    std::thread t1(producer);
    std::thread t2(consumer);
    std::thread t3(consumer);
    t1.join();
    t2.join();
    t3.join();
}
}

///////////////////////////////////////////////////////////////////////////////
// Semaphore
// Counting semaphore: multiple threads can access the shared resource, but only
// up to a given number of threads that the semaphore counts. Aquiring the semaphore
// decreases the count, and releasing the semaphore increases the count. A count of
// zero means the semaphore cannot be aquired.
// 
// Binary semaphore: Like a mutex. A mutex can only be unlocked by the same thread
// that locked in. This restriction doesn't exist for semaphores.
//
// Can be used to synchronize actions between threads. 
// The typical use of a semephore is to track the use of a limited resource.
// The semaphore itself requires mutual exclusion when adjusting its iternal state.
///////////////////////////////////////////////////////////////////////////////

namespace SemaphoreDemo {
// A semaphore can be implemented using a mutex and a condition variable.
class Semaphore {
public:
    Semaphore(unsigned long initCount) {
        mCount = initCount;
    }

    void acquire() { // decrement the internal counter
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mCount) {
            mCondVar.wait(lock);
        }
        mCount--;
    }

    void release() { // increment the internal counter
        std::unique_lock<std::mutex> lock(mMutex);
        mCount++;
        lock.unlock();
        mCondVar.notify_one();
    }

private:
    std::mutex mMutex;
    std::condition_variable mCondVar;
    unsigned long mCount;
};

// If you make the initial count=1, it becomes a binary semaphore.
Semaphore semaphore(4);

void task(int id) {
    // the counting semaphore only allows 4 tasks can run at once
    semaphore.acquire(); 
    printf("Task %d is running...\n", id);
    srand(id); // seed random number generator
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 2000 + 1000));
    printf("Task %d is DONE running\n", id);
    semaphore.release();
}

void semaphoreDemo() {
    std::thread threads[10];
    for (int i = 0; i < 10; i++) {
        threads[i] = std::thread(task, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Race Condition
// When the outcome of the program changes due to how threads are scheduled.
// Different to a data race where errors occur due to read/write access to shared 
// respurce at the same time. Data races can detect easier using code analysis tools.
/////////////////////////////////////////////////////////////////////////////////////////

namespace RaceConditionDemo {
unsigned int result = 1;
std::mutex mut;

void cpuWork(unsigned long workUnits) {
    unsigned long x = 0;
    for (unsigned long i = 0; i < workUnits*1000000; i++) {
        x++;
    }
}

void worker1() {
    cpuWork(1); // do a bit of work first
    std::scoped_lock<std::mutex> lock(mut);
    result *= 2;
    printf("result doubled.\n");
}

void worker2() {
    cpuWork(1); // do a bit of work first
    std::scoped_lock<std::mutex> lock(mut);
    result += 3;
    printf("result + 3.\n");
}

void raceConditionDemo() {
    std::thread threads[10];
    for (int i=0; i<10; i+=2) {
        threads[i] = std::thread(worker1);
        threads[i+1] = std::thread(worker2);
    }
    for (auto& s : threads) {
        s.join();
    }
    printf("The result is %u.\n", result);
}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Barriers
// Here all threads must wait at the barrier until all have arrived. Then they all proceed.
// We use it here to enforce the additions before the multiplications.
/////////////////////////////////////////////////////////////////////////////////////////

namespace BarrierDemo {

unsigned int result = 1;
std::mutex mut;
std::barrier addBarrier(10);

void cpuWork(unsigned long workUnits) {
    unsigned long x = 0;
    for (unsigned long i = 0; i < workUnits*1000000; i++) {
        x++;
    }
}

void worker1() {
    cpuWork(1); // do a bit of work first
    addBarrier.arrive_and_wait();  // The barrier forces threads to wait until the number specified (10) arrive.
    std::scoped_lock<std::mutex> lock(mut);
    result *= 2;
    printf("result doubled.\n");
}

void worker2() {
    cpuWork(1); // do a bit of work first
    {
        std::scoped_lock<std::mutex> lock(mut);
        result += 3;
    }
    printf("result + 3.\n");
    addBarrier.arrive_and_wait();
}

void barrierDemo() {
    std::thread threads[10];
    for (int i = 0; i < 10; i += 2) {
        threads[i] = std::thread(worker1);
        threads[i+1] = std::thread(worker2);
    }
    for (auto &s : threads) {
        s.join();
    }
    printf("The result is %u.\n", result);
}

}


/////////////////////////////////////////////////////////////////////////////////////////
// Latches
// The difference here is that the latch can differentiate between threads that need to wait
// and threads that just need to signal that they have done. In this example, the adder threads
// do not need to wait for the multiplication threads so they just signal using the count_down()
// function that they are done and they immediately move on. Once all the adder threads signal,
// the waiting multiplication threads can start.
//
// Can use the latch as a gate, one controlling thread monitors a condition to allow all
// waiting threads to continue.
/////////////////////////////////////////////////////////////////////////////////////////

namespace LatchDemo {
unsigned int result = 1;
std::mutex mut;
std::latch addLatch(5);

void cpuWork(unsigned long workUnits) {
    unsigned long x = 0;
    for (unsigned long i = 0; i < workUnits*1000000; i++) {
        x++;
    }
}

void worker1() {
    cpuWork(1); // do a bit of work first
    addLatch.wait();    // Have to wait here to the 5 worker2 threads are finished
    std::scoped_lock<std::mutex> lock(mut);
    result *= 2;
    printf("result doubled.\n");
}

void worker2() {
    cpuWork(1); // do a bit of work first
    {
        std::scoped_lock<std::mutex> lock(mut);
        result += 3;
    }
    printf("result + 3.\n");
    addLatch.count_down();
}

void latchDemo() {
    std::thread threads[10];
    for (int i = 0; i < 10; i += 2) {
        threads[i] = std::thread(worker1);
        threads[i+1] = std::thread(worker2);
    }
    for (auto &s : threads) {
        s.join();
    }
    printf("The result is %u.\n", result);
}

}

///////////////////////////////////////////////////////////////////////////////
// Thread Pools
// Allow for the reduction in thread creation overhead by creating a fixed pool
// of threads that execute a list of tasks. When one thread is finished it takes
// another task from the list of tasks that have been posted to the pool.
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_BOOST

namespace ThreadPoolDemo {

void task(int taskID) {
    size_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
    printf("Thread %llu execute task %d.\n", threadID, taskID);
}

void threadPoolDemo() {
    boost::asio::thread_pool pool(4);
    for (int i = 0; i < 100; i++) {
        boost::asio::post(pool, [i](){ task(i); });
    }
    pool.join();
}

} // namespace ThreadPoolDemo

#endif

///////////////////////////////////////////////////////////////////////////////
// Futures
// Creates an ansynchronous task that will return a value. When the calling thread
// needs it, it uses a get on the future. If the return value is available, the
// future immediately returns, else the calling thread will wait until the future
// terminates.
///////////////////////////////////////////////////////////////////////////////

namespace FutureDemo {

int countingFuture() {
    printf("Future is counting...\n");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 42;
}

void futureDemo() {
    std::future<int> result = std::async(std::launch::async, countingFuture);
    printf("Main thread is doing other things...\n");
    printf("Counting future count is %d\n", result.get());
}

} // namespace FutureDemo

///////////////////////////////////////////////////////////////////////////////
// Divide & Conquer Type Algorithm Example
//
// Need to set the number of async tasks closer to the number of parallel tasks
// that can actually execute (number of cores), otherwise you'll have a large 
// overhead of creating threads that just sit and wait most of the time.
///////////////////////////////////////////////////////////////////////////////

namespace DivideAndConquerDemo {

std::uint64_t recursiveSum(std::uint32_t lo, std::uint32_t hi, std::uint32_t depth=0) {
    if (depth > 3) { // base case threshold
        std::uint64_t sum = 0;
        for (auto i = lo; i < hi; i++) {
            sum += i;
        }
        return sum;
    } else { // divide and conquer
        auto mid = (hi + lo) / 2; // middle index for splitting
        auto left = std::async(std::launch::async, recursiveSum, lo, mid, depth+1);
        //auto left = recursiveSum(lo, mid, depth+1)
        auto right = recursiveSum(mid, hi, depth+1);
        return left.get() + right;
    }
}

void divideAndConquerDemo() {
    std::uint64_t total = recursiveSum(0, 1000000000);
    printf("Total: %lld\n", total);
}

} // namespace DivideAndConquerDemo

int main() {
    ConditionVariableBusyWaitingDemo::conditionVariableDemo();
    ConditionVariableDemo::conditionVariableDemo();
    PipelineDemo::pipelineExample();
    SemaphoreDemo::semaphoreDemo();
    RaceConditionDemo::raceConditionDemo();
    BarrierDemo::barrierDemo();
    LatchDemo::latchDemo();
    #ifdef USE_BOOST
    ThreadPoolDemo::threadPoolDemo();
    #endif
    FutureDemo::futureDemo();
    DivideAndConquerDemo::divideAndConquerDemo();
}


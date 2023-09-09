#include <cstdio>
#include <iostream>
#include <thread>       // std::thread, std::this_thread
#include <chrono>       // std::chrono
#include <unistd.h>     // getpid()
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <sstream>
#include <array>

///////////////////////////////////////////////////////////////////////////////
// Thread demo.
///////////////////////////////////////////////////////////////////////////////

// A simple function that wastes CPU cycles "forever".
void cpuWaster() {
    std::cout << "CPU Waster Process ID: " << getpid() << std::endl;
    std::cout << "CPU Waster Thread ID " << std::this_thread::get_id() << std::endl;
    while (true) continue;
}

// Starts two threads which print their ID numbers.
void threadDemoMain() {
    std::cout << "Main Process ID: " << getpid() << std::endl;
    std::cout << "Main Thread ID: " << std::this_thread::get_id() << std::endl;
    std::thread thread1(cpuWaster);
    std::thread thread2(cpuWaster); 

    while (true) { // keep the main thread alive "forever"
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

///////////////////////////////////////////////////////////////////////////////
// Demo of the schedular giving different runtimes to each thread.
///////////////////////////////////////////////////////////////////////////////

// This global is shared memory between threads.
bool runThreadFlag = true;

// This function simply counts the number of loop iterations it performs before it is told
// to terminate.
void incrementer(const char* name) {
    unsigned int count = 0;
    while (runThreadFlag) {
        count++;
    }
    std::printf("%s counted to %u\n", name, count);
}

// Starts two threads, waits a second, and then flags both threads to end.
void schedulerMain() {
    std::thread thread1(incrementer, "Thread1");
    std::thread thread2(incrementer, "Thread2");
    
	printf("Thread1 and Thread2 are counting...\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    runThreadFlag = false;
    thread1.join();
    thread2.join();
}

///////////////////////////////////////////////////////////////////////////////
// Example of a threads lifecycle.
///////////////////////////////////////////////////////////////////////////////

void sleepyThread() {
    std::printf("Sleepy thread started & sleeps for three seconds...\n");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::printf("Sleepy thread is done.\n");
}

void lifeCycleMain() {
    printf("Main requests sleepy thread's help.\n");
    std::thread sleepy(sleepyThread);
    printf("  Sleepy is joinable? %s\n", sleepy.joinable() ? "true" : "false");

    printf("Main continues cooking soup.\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    printf("  Sleepy is joinable? %s\n", sleepy.joinable() ? "true" : "false");

    printf("Main patiently waits for sleepy to finish and join...\n");
    sleepy.join();
    printf("  Sleepy is joinable? %s\n", sleepy.joinable() ? "true" : "false");

    printf("Main and sleepy are both done!\n");
}

///////////////////////////////////////////////////////////////////////////////
// Demo using daemon threads.
///////////////////////////////////////////////////////////////////////////////

void thisIsADaemonThread() {
    while (true) {
        printf("This is a daemon thread.\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void daemonThreadDemoMain() {
    std::thread aDaemon(thisIsADaemonThread);
    aDaemon.detach();   // Detaching a thread makes it a daemon thread.
    for (int i=0; i<3; i++) {
        // printf is faster than cout<< so it is better for these examples.
        printf("This is the main thread...\n");     
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
    printf("The main thread is done!\n");
    // do not use join() on a detached thread
    // When this thread finishes, the daemon will automatically be terminated.
}

///////////////////////////////////////////////////////////////////////////////
// Data race example.
///////////////////////////////////////////////////////////////////////////////

/*
- Use tools valgrind, (cppcheck doesn't appear to check this.)
- https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual
- Examine a data source that is referenced by two or more threads.
*/

unsigned int count = 0;

void counter() {
    for (int i = 0; i < 10000000; i++) {
        count++;
    }
}

void dataRaceExample() {
    std::thread counter1(counter);
    std::thread counter2(counter);
    counter1.join();
    counter2.join();
    printf("The counter should be 20000000, it is %u.\n", count);
}

///////////////////////////////////////////////////////////////////////////////
// Mutex example. Same as above but with a mutex.
///////////////////////////////////////////////////////////////////////////////

unsigned int countForMutexExample = 0;
std::mutex counterLock;

void counterWithLock() {
    for (int i = 0; i < 5; i++) {
        printf("Long IO operations...\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        counterLock.lock();
        countForMutexExample++;
        counterLock.unlock();
    }
}

void mutexExample() {
    std::thread counter1(counterWithLock);
    std::thread counter2(counterWithLock);
    counter1.join();
    counter2.join();
    printf("The counter should be 10, it is %u.\n", countForMutexExample);
}

///////////////////////////////////////////////////////////////////////////////
// Atomic operation example.
///////////////////////////////////////////////////////////////////////////////

std::atomic<unsigned int> atomicCount(0);

void atomicCounter() {
    for (int i = 0; i < 10000000; i++) {
        atomicCount++;
    }
}

void atomicExample() {
    std::thread counter1(atomicCounter);
    std::thread counter2(atomicCounter);
    counter1.join();
    counter2.join();
    printf("The counter should be 20000000, it is %u.\n", atomicCount.load());
}

///////////////////////////////////////////////////////////////////////////////
// Re-entrant mutex example.
///////////////////////////////////////////////////////////////////////////////

unsigned int count1 = 0;
unsigned int count2 = 0;
std::recursive_mutex theLock;

void incrementCount1() {
    theLock.lock();
    count1++;
    theLock.unlock();
}

void incrementCount2() {
    theLock.lock();
    count2++;
    incrementCount1();      // This calls the mutex twice which causes a deadlock.
                            // This needs to be a recursive_mutex.
    theLock.unlock();
}

void doubleIncrementer() {
    for (int i = 0; i < 10000; i++) {
        incrementCount1();
        incrementCount2();
    }
}

void recursiveMutexExample() {
    std::thread thread1(doubleIncrementer);
    std::thread thread2(doubleIncrementer);
    thread1.join();
    thread2.join();
    printf("Count1 is %u.\n", count1);
    printf("Count2 is %u.\n", count2);
}

///////////////////////////////////////////////////////////////////////////////
// Try lock example.
///////////////////////////////////////////////////////////////////////////////

namespace TryLockExample {
unsigned int count = 0;
std::mutex lock;

void incrementer(const char* name) {
    int threadCount = 0;
    while (count <= 20) {
        if (threadCount && lock.try_lock()) { // add local counts to shared count variable.
            count += threadCount;
            printf("%s added %u to count.\n", name, threadCount);
            threadCount = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(300)); // time of IO operation
            lock.unlock();
        } else { // count on local variable
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // time of IO operation
            threadCount++;
            printf("%s added 1 to local count.\n", name);
        }
    }
}

void tryLockExample() {
    auto start_time = std::chrono::steady_clock::now();
    std::thread thread1(incrementer, "Thread1");
    std::thread thread2(incrementer, "Thread2");
    thread1.join();
    thread2.join();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
    printf("Elapsed Time: %.2f seconds\n", elapsed_time/1000.0);
}
}   // namespace TryLockExample

///////////////////////////////////////////////////////////////////////////////
// Shared mutex demo. Can share a lock with multiple threads if they are not
// modifying the data. Else, a writer thread can request exclusion.
//
// With a normal lock, all reader threads must wait to read the data. 
// With a shared_mutex, the reader threads can all run concurrently.
///////////////////////////////////////////////////////////////////////////////

namespace SharedMutexDemo {

char WEEKDAYS[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int today = 0;
std::shared_mutex marker;

void calendar_reader(const int id) {
    for (int i=0; i<7; i++) {
        marker.lock_shared();
        printf("Reader-%d sees today is %s\n", id, WEEKDAYS[today]);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        marker.unlock_shared();
    }
}

void calendar_writer(const int id) {
    for (int i=0; i<7; i++) {
        marker.lock();
        today = (today + 1) % 7;
        printf("Writer-%d updated date to %s\n", id, WEEKDAYS[today]);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        marker.unlock();        
    }
}

void sharedMutexDemo() {
    // create ten reader threads ...but only two writer threads
    std::array<std::thread, 10> readers;
    for (unsigned int i=0; i<readers.size(); i++) {
        readers[i] = std::thread(calendar_reader, i);
    }
    std::array<std::thread, 2> writers;
    for (unsigned int i=0; i<writers.size(); i++) {
        writers[i] = std::thread(calendar_writer, i);
    }

    // wait for readers and writers to finish
    for (unsigned int i=0; i<readers.size(); i++) {
        readers[i].join();
    }
    for (unsigned int i=0; i<writers.size(); i++) {
        writers[i].join();
    }
}
} // namespace SharedMutexDemo


///////////////////////////////////////////////////////////////////////////////
// Deadlock demo.
// Make sure that both threads lock the mutex in the same order.
// Prevent deadlocks
//  - Lock Ordering (not always possible)
//  - scoped_lock
///////////////////////////////////////////////////////////////////////////////

namespace DeadlockDemo {
int inventory = 5000;

void consumer(std::mutex &lockA, std::mutex &lockB) {
    // In reality, only one lock is required for this single shared piece of data.
    while (inventory > 0) {
        //lockA.lock();
        //lockB.lock();
        std::scoped_lock lock(lockA, lockB);    // has deadlock avoidance logic.
        if (inventory) {
            inventory--;
        }
        // lockB.unlock();
        // lockA.unlock();
    }
}

void deadlockDemo() {
    std::mutex lockA, lockB;
    // If you swap the lock order in one of these threads you'll likely get deadlock.
    // Though a scoped lock helps to avoid deadlock when the order is swapped.
    std::thread thread1(consumer, std::ref(lockA), std::ref(lockB));
    std::thread thread2(consumer, std::ref(lockB), std::ref(lockA));
    thread1.join();
    thread2.join();
    std::printf("There is no inventory left.\n");
}
} // namespace DeadlockDemo

///////////////////////////////////////////////////////////////////////////////
// Abandoned lock demo. Happens when the thread terminates before unlocking
// a mutex.
//  - use a scoped_lock to unlock the mutex when the thread leaves scope.
///////////////////////////////////////////////////////////////////////////////

namespace AdandonedlockDemo {
int inventory = 5000;

void consumer(std::mutex &lock) {
    while (inventory > 0) {
        std::scoped_lock scopedLock(lock);
        if (inventory) {
            inventory--;
        }
        if (inventory == 10) {
            printf("Abandoning the lock...\n");
            break;
        }
    }
}

void abandonedLockDemo() {
    std::mutex lock;
    // If you swap the lock order in one of these threads you'll likely get deadlock.
    // Though a scoped lock helps to avoid deadlock when the order is swapped.
    std::thread thread1(consumer, std::ref(lock));
    std::thread thread2(consumer, std::ref(lock));
    thread1.join();
    thread2.join();
    std::printf("There is no inventory left.\n");
}
} // namespace AdandonedlockDemo

///////////////////////////////////////////////////////////////////////////////
// Starvation.
///////////////////////////////////////////////////////////////////////////////

namespace StarvationDemo {
int inventory = 5000;

void consumer(std::mutex &lock) {
    int inventoryTaken = 0;
    while (inventory > 0) {
        std::scoped_lock scopedLock(lock);
        if (inventory) {
            inventory--;
            inventoryTaken++;
        }
    }
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    printf("Thread %s took %d items.\n", oss.str().c_str(), inventoryTaken);
}

void starvationDemo() {
    std::mutex lock;
    // Because there are so many threads, some of these don't get an opportunity to access
    // the shared resource before the process terminates.
    std::array<std::thread, 200> consumers;
    for (size_t i = 0; i < consumers.size(); i++) {
        consumers[i] = std::thread(consumer, std::ref(lock));
    }
    for (size_t i = 0; i < consumers.size(); i++) {
        consumers[i].join();
    }
    std::printf("There is no inventory left.\n");
}
} // namespace StarvationDemo

///////////////////////////////////////////////////////////////////////////////
// Live lock.
///////////////////////////////////////////////////////////////////////////////

namespace LivelockDemo {
int inventory = 5000;

void consumer(std::mutex &lockA, std::mutex &lockB) {
    while (inventory > 0) {
        lockA.lock();
        // This if statement tries to be polite and if it can't get the second lock,
        // it returns the first lock.
        if (!lockB.try_lock()) {
            lockA.unlock();
            // makes thread wait some time before trying again.
            std::this_thread::yield();
        } else {
            if (inventory) {
                inventory--;
            }
            lockB.unlock();
            lockA.unlock();
        }
    }
}

void livelockDemo() {
    std::mutex lockA, lockB;
    // A livelock will consume CPU consumption as opposed to a deadlock where all threads go to sleep.
    std::thread thread1(consumer, std::ref(lockA), std::ref(lockB));
    std::thread thread2(consumer, std::ref(lockB), std::ref(lockA));
    std::thread thread3(consumer, std::ref(lockA), std::ref(lockB));
    std::thread thread4(consumer, std::ref(lockB), std::ref(lockA));
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    std::printf("There is no inventory left.\n");
}
} // namespace LivelockDemo


int main() {
    // threadDemoMain(); // This has an infinite loop
    schedulerMain();
    lifeCycleMain();
    daemonThreadDemoMain();
    dataRaceExample();
    mutexExample();
    atomicExample();
    recursiveMutexExample();
    TryLockExample::tryLockExample();
    SharedMutexDemo::sharedMutexDemo();
    DeadlockDemo::deadlockDemo();
    AdandonedlockDemo::abandonedLockDemo();
    StarvationDemo::starvationDemo();
    LivelockDemo::livelockDemo();
    return 0;
}

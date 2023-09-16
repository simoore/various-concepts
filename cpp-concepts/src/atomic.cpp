#include <atomic>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMPARE AND EXCHANGE
// --------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace compareandexchange {

void run(void) {
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

int main() {
    compareandexchange::run();
}
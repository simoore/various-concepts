#include <bitset>
#include <stdexcept>
#include <iostream>

int main() {

    // The constructor parameter can only have characters "0" and "1"
    try {
        std::bitset<4>{"012"};
    } catch (std::invalid_argument const &ex) {
        std::cout << "1. " << ex.what() << std::endl;
    }
}
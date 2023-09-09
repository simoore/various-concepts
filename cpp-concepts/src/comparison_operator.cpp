// Rather than defining the operators { == != < <= >= > } seperately, C++20 provides a few features to make things
// more succinct.
//
// 1. == implies != so you only need to define the == operator.
// 2. There is the <=> operator for handling all cases of <. <=, >=, and >

// To use <=> in expression you need the <compare> header.
#include <compare>
#include <iostream>

class Value1 {
private:
    long id;

public:
    Value1(long i) noexcept : id{i} {}

    // The default operation will compare member by member. Note that order of member variables matter.
    // This default definition also includes the == operator.
    auto operator<=>(const Value1 &rhs) const = default;
};

class Value2 {
private:
    long id;

public:
    Value2(long i) noexcept : id{i} {}


    // Returns int, 0 is equality, lhs < rhs return is negative, lhs > rhs return is positive
    auto operator<=>(const Value2 &rhs) const {
        return id <=> rhs.id;
    };

    auto operator==(const Value2 &rhs) const {
        return id == rhs.id;
    }
};

class Value3 {
private:
    long id;

public:
    Value3(long i) noexcept : id{i} {}


    // Returns int, 0 is equality, lhs < rhs return is negative, lhs > rhs return is positive
    auto operator<=>(const Value3 &rhs) const {
        if (id == rhs.id) {
            return std::strong_ordering::equal;
        } else if (id < rhs.id) {
            return std::strong_ordering::less;
        } else {
            return std::strong_ordering::greater;
        }
    };

    auto operator==(const Value3 &rhs) const {
        return id == rhs.id;
    }
};

int main() {
    // The following operators don't return a bool value due to the definitions of the comparison operator in the 
    // classes above. They return an enumeration object that indicates the type of ordering (strong, weak, partial),
    // and the result of the comparison. For integral types strong ordering is used. 
    std::cout << static_cast<bool>(Value1{1} == Value1{3}) << std::endl;
    std::cout << static_cast<bool>(Value2{1} < Value2{3}) << std::endl;

    bool ans = (Value3{11} <=> Value3{13}) == std::strong_ordering::less;
    std::cout << ans << std::endl;
}
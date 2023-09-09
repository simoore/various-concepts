// lvalues and rvalues in C++
// https://www.youtube.com/watch?v=fbYknr-HPYE
//
// Move Semantics in C++
// https://www.youtube.com/watch?v=ehMg6zvXuMY
//
// You can assign either an rvalue or lvalue to an lvalue. lvalues have explicity storage associated with them.
// You cannot assign anthing to an rvalue (it is temporary)
// Only lvalues can be assigned to an lvalue reference, and only rvalues can be assigned to rvalue references. This
// allows a function to distinguish between cases where storage has been allocated for a variable for use elsewhere,
// or if the value is temporary and it's resources will disappear after the function is called. This allows some
// optimization of resources to be performed using move semantics. 
// Normal function parameters are lvalues.
//
// The reason we want to use rvalues is that there is no ownership of its resources. If we pass an rvalue to a function
// we can take the resources assocuated with that object and use it for something else as we know the value is 
// temporary.

#include <iostream>

// This is returning a rvalue.
int getValue() {
    return 10;
}

// This is returning a lvalue reference. It has storage associated with the return type.
int &getValueRef() {
    static int value = 10;
    return value;
}

// This parameter is an lvalue. It will be assigned to by either an lvalue or rvalue when the function is called.
// There is memory associated with the lvalue parameter.
void setValue(int value) {
    value++;
    std::cout << value << std::endl;
}

void setValueRef(int &value) {
    value++;
    std::cout << "[lvalue]: " << value << std::endl;
}

// This how we can accept temporary rvalues into a function. We use an rvalue reference. This allows us to distinguish
// between an lvalue reference that 
void setValueRef(int &&value) {
    std::cout << "[rvalue]: " << value << std::endl;
}

void printString(std::string &str) {
    std::cout << "[lvalue]: " << str << std::endl;
}

void printString(std::string &&str) {
    std::cout << "[rvalue]: " << str << std::endl;
}

int main() {
    // rvalue are temporary values, they have no allocated address or storage. These include literal constants and 
    // return values. rvalues can never be on the LHS of the assignment. In this expression i is an lvalue and 10 is 
    // an rvalue.
    int i = 10;

    // getValueRef returns a modifiable lvalue reference, thus this kind of expression is allowed.
    getValueRef() = 12; 
    std::cout << getValueRef() << std::endl;

    // We can call setvalue from either a lvalue or rvalue. The value parameter of the setValue() function is an lvalue.
    // It will have memory allocated and an assignment operation will take place from the passed in lvalue or rvalue.
    setValue(i);
    setValue(11);

    // You cannot make an lvalue reference from an rvalue, thus only lvalue can be passed to this setValueRef. However,
    // if it is a const lvalue reference, we can assign it an rvalue as the compiler will allocate storage for it. So
    // if the input to setValueRef was a const lvalue reference we could pass it literals directly like setValue.
    setValueRef(i);
    const int &a = 10;
    static_cast<void>(a);

    // If we create an overloaded function that accepts an rvalue, we can know pass the temporary rvalue to a function.
    setValueRef(13);

    // The result of intermediate computations is also an rvalue. For example, here str1, str2, and concat are lvalues
    // while the two literal strings and the expression (str1 + str2) are rvalues. We can call the two overloaded 
    // versions of the print string function - one using lvalue references and one using rvalue references using 
    // the lvalue concat, and the rvalue (str1 + str2).
    std::string str1 = "This ";
    std::string str2 = "is a string.";
    std::string concat = str1 + str2;
    printString(concat);
    printString(str1 + str2);
}
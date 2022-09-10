// Folds reduce a list into a single variable. This example implements a compile time version of the all() function.
// all() takes a list of bools (or values that can be cast to bools) and returns true if all bools in the list are 
// true, else it returns false.
//
// https://www.modernescpp.com/index.php/from-variadic-templates-to-fold-expressions

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
// VARIADIC TEMPLATE APPROACH
///////////////////////////////////////////////////////////////////////////////

// This is the terminating expression for allVar() when an empty parameter pack is passed to allVar() in the 
// recusive expression below.
bool allVar() {
    return true;
}

// This function takes a list of bools and returns true if they are all true otherwise false.
//
// Parameter Packs
// ---------------
// A parameter pack can be thought of as a list of types or values.
// int... I : I is a parameter pack of ints. You can unpack I with the syntax I...
// If declared as a template parameter, it is a template parameter pack. If declared as a function parameter, it is a
// function parameter pack.
// Parameter packs cannot be operated on at runtime.
//
// https://anderberg.me/2016/08/01/c-variadic-templates/
//
// @param t
//  The first element in the list.
// @param ts
//  The tail of the list.
template <typename T, typename... Ts>
bool allVar(T t, Ts... ts) {
    return t && allVar(ts...);
}

///////////////////////////////////////////////////////////////////////////////
// C++ FOLD EXPRESSION
///////////////////////////////////////////////////////////////////////////////

// This approach uses the fold operator that has been included since C++17.
//
// https://en.cppreference.com/w/cpp/language/fold
//
// The operator syntax for the unary left fold is (... op pack). In this case && is the operator. The unary left fold
// is eqivalent to this expression:
//
// (... op P) = (((P1 op P2) op ...) op PN)
//
// That is the operator is applied to the start of the list first. A unary right operator applies to the end of the
// list first. There is also binary folds that allow for an initial value to be specified as the first operand.
//
// We don't use bool... because the type(s) make be able to cast to a bool.
template <typename... Ts>
bool allFold(Ts... ts) {
    return (... && ts);
}

///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////

int main() {
    std::cout << std::boolalpha;

    std::cout << std::endl;

    std::cout << "allVar(): " << allVar() << std::endl;
    std::cout << "allFold(): " << allFold() << std::endl;

    std::cout << "allVar(true): " << allVar(true) << std::endl;
    std::cout << "allFold(true): " << allFold(true) << std::endl;

    std::cout << "allVar(true, true, true, false): " << allVar(true, true, true, false) << std::endl;
    std::cout << "allFold(true, true, true, false): " << allFold(true, true, true, false) << std::endl;

    std::cout << std::endl;
}
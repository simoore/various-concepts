// References
// ----------
// CppCon 2014: Walter E. Brown "Modern Template Metaprogramming: A Compendium, Part I
// https://www.youtube.com/watch?v=Am2is2QCvxY
//
// CppCon 2014: Walter E. Brown "Modern Template Metaprogramming: A Compendium, Part II
// https://www.youtube.com/watch?v=a0FliKwcwXE
//
// This website shows you how a compiler expresses certain C++ concepts with a more basic syntax.
// https://cppinsights.io/
//
// Shows the assembly produced by various compilers.
// https://godbolt.org/

#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

// Absolute value of template parameter N. 
template <int N>
struct Abs {
    static_assert(N != std::numeric_limits<int>::min(), "Cannot represent Abs of this number.");
    static constexpr int value = (N < 0) ? -N : N;
};

// constexpr functions provide compile time evaluation with a more familar syntax. constexpr functions cannot operate
// on types.
constexpr int absConst(int N) {
    return (N < 0) ? -N : N;
}

// Compile time recursion to calculate greatest common divisor from Euclid. Requires the specialization in the next
// struct to terminate the specialization.
template <unsigned M, unsigned N>
struct gcd {
    static constexpr int value = gcd<N, M%N>::value;
};

template<unsigned M>
struct gcd<M, 0> {
    static_assert(M != 0, "gcd of 0 and 0 undefined.");
    static int const value = M;
};

// The rank is the dimension of a data type. If it is not a container or array, the rank is zero. For every dimension
// of the array the rank increases by 1. For example int[10][20][30] is rank 3.
template <typename T>
struct rank {
    static constexpr size_t value = 0;
};

// This specialization will capture array types.
template <typename U, size_t N>
struct rank<U[N]> {
    static constexpr size_t value = 1u + rank<U>::value;
};

// A metafunction can return types. This is an implementation of removing const from a type.
template <typename T>
struct RemoveConst {
    using type = T;
};

template <typename T>
struct RemoveConst<const T> {
    using type = T;
};

// Can use inheritance to provide the fields of a struct to another struct. For example, above both the structs have
// ths same field `using type = T`. We could also express this template metafunction this way:
template <typename T>
struct TypeIs {
    using type = T;
};

template <typename T>
struct RemoveConst2: TypeIs<T> {};

template <typename T>
struct RemoveConst2<const T>: TypeIs<T> {};

// Compile time decision making. The primary template handles the bool is true case. We just return the true class
// in this case. The specialization handles the false case. This is how the std::conditional works.
template <bool, typename T, typename>
struct If : TypeIs<T> {};

template <typename T, typename F>
struct If<false, T, F>: TypeIs<F> {};

// SFINAE: Substitution failure is not an error. In this example we can create two template functions with what
// appears to be the same definition. But the return type is expressec by an enable if metafunction. Functions must
// have a return type so when the enable if is empty, the template will be ill-formed and discared allowing the 
// other template metafunction to be used. In C++20, concepts have superceeded this approach.
template <typename T>
std::enable_if_t<std::is_integral<T>::value, std::string> f(T val) {
    return "This is an integral type.";
}

template <typename T>
std::enable_if_t<std::is_floating_point<T>::value, std::string> f(T val) {
    return "This is an floating point type.";
}

// Let's implement IsVoid
template <typename T> struct IsVoid: std::false_type {};
template <> struct IsVoid<void>: std::true_type {};
template <> struct IsVoid<const void>: std::true_type {};
template <> struct IsVoid<volatile void>: std::true_type {};
template <> struct IsVoid<const volatile void>: std::true_type {};

// Let's implement IsSame. The primary template handles when the two types are different, and the specialization 
// handles when they are the same.
template <typename T, typename U> struct IsSame: std::false_type {};
template <typename T> struct IsSame<T, T>: std::true_type {};

///////////////////////////////////////////////////////////////////////////////
// IsOneOf
///////////////////////////////////////////////////////////////////////////////

// Is type T in Ts. First declare the template. We will implement this with specialization. The declaration saids that
// this template takes one or more type as the parameter pack can be empty.
template <typename T, typename... Ts>
struct IsOneOf;

// If no items in the list, the result is false.
template <typename T>
struct IsOneOf<T>: std::false_type {};

// If the type T is at the head of the list, we return true. For a specialization, the template parameters define
// what can exist, while the template parameters of the specialization are for pattern matching.
template <typename T, typename... Ts>
struct IsOneOf<T, T, Ts...>: std::true_type {};

// If if type T1 is not at the start of the list, we discard the first element of the list and recall the metafunction.
template <typename T1, typename T2, typename... Ts>
struct IsOneOf<T1, T2, Ts...>: IsOneOf<T1, Ts...> {};

///////////////////////////////////////////////////////////////////////////////
// Unevaluated Operands
///////////////////////////////////////////////////////////////////////////////

// With decltype, we inspect the expression and determine what its type is - we don't evaluated the expression.
// Thus a decltype only needs the declaration.

bool aFunc(std::string str) {
    std::cout << str << std::endl;
    return 1.0;
}

int aFunc(float a) {
    return static_cast<int>(a);
}

// Need the declval to insert a type into the function so the compiler can figure out which overloaded function to use.
using aFuncRetType1 = decltype(aFunc(std::declval<float>()));       // Is int
using aFuncRetType2 = decltype(aFunc(std::declval<std::string>())); // Is bool

// Determines if the copy assignment operator is defined for a type.
template <typename T>
struct IsCopyAssignable {
private:

    // This an application of SFINAE, if the assignment operator doesn't exist for type U, then this template instance
    // will be ill-formed and discarded. It is all to do with the second template parameter. We don't refer to it in 
    // the declaration so we don't give it a name. We give iy a default value. It is the return type of the copy 
    // assignment operator. U& = connst U& can only invoke the copy assignment since we are not able to modify the RHS 
    // since it is const lvalue reference. If we could modify it or it was an rvalue reference we could do a 
    // move assignment instead.
    template <typename U, typename = decltype( std::declval<U&>() = std::declval<const U&>() )>
    static std::true_type tryAssignment(U&&);

    // If the overload resolution doesn't fit the declaration of tryAssignment above, this will catch all other 
    // uses of tryAssignment. The ... syntax is like the last resort function call. This is the default answer.
    static std::false_type tryAssignment(...);

public:

    // We will inspect tryAssignment when called with the type T. decltype then gives us the return type of the call.
    // Depending on which of the two functions above are used when the compiler performs the overloading resolution,
    // type will either be std::true_type or std::false_type.
    using type = decltype(tryAssignment(std::declval<T>()));
};

///////////////////////////////////////////////////////////////////////////////
// void_t: Maps well defined types to void. 
///////////////////////////////////////////////////////////////////////////////

// This maps a list of well defined types to void. If one of the types are not well defined, SFINAE removes the 
// definition of of void_t<Ts...>
template <typename... Ts>
using void_t = void;

// Primary template, this
template <typename, typename = void>
struct HasTypeMember: std::false_type {};

// In the specialization, the second template parameter is well-defined if T::type exists, otherwise it is ill-formed
// and idiscarded due to SFINAE and the primary template catches it.
template <typename T>
struct HasTypeMember<T, void_t<typename T::type>>: std::true_type {};

// https://stackoverflow.com/questions/27687389/how-does-void-t-work
//
// 1. When HasTypeMember is used, the compiler looks for the primary template. Say you calle it like HasTypeMember<A>,
// the compiler will match it as if you called HasTypeMember<A, void>
//
// 2. Now the template parameters are compared to any specialization of HasTypeMember. There is one specialization.
// First the template parameters must be deduced. So a specialization from the above example becomes
// template <>
// HasTypeMember<A, void_t<typename A::type>>
//
// If member type exists in A, the specialization becomes: template <> HasTypeMember<A, void> and since that 
// matches the template parameters of the primary template, this specialization is used. If type does not exist
// the specialization evaluates to HasTypeMemeber<A, SFINAE> which is ill-formed and is discard and the primary
// template is used as the fallback.

///////////////////////////////////////////////////////////////////////////////
// IsCopyAssignable2 with void_t
///////////////////////////////////////////////////////////////////////////////

template <typename T> 
using CopyAssignment_t = decltype(std::declval<T&>() = std::declval<const T&>());

// Primary template, the fallback if the specialization doesn't match which determines if the type is copy assignable.  
template <typename T, typename = void>
struct IsCopyAssignable2: std::false_type {};

// First there is a SFINAE expression that must pass for this specialization to be used. First, the void_t<> type
// needs to be well-formed. First thing to note is that CopyAssignment_t<T> is well-formed if operator= is defined as 
// this is the function we are inspecting in its decltype. Then the second template parameter is well-formed if the 
// the CopyAssignment_t type is well-formed for a given type. If this passes we use this specialization.
//
// However, there is one additional condition that determines if this expression is copy assignable. operator= must 
// return T&. That is why the inherited struct is_same rather than true_type to add in this additional requirement.
template <typename T>
struct IsCopyAssignable2<T, void_t<CopyAssignment_t<T>>>: std::is_same<CopyAssignment_t<T>, T&> {};

///////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////

int main() {
    std::cout << Abs<-5>::value << std::endl;

    static constexpr int val = absConst(-7);
    std::cout << val << std::endl;

    std::cout << gcd<24, 16>::value << std::endl;

    using array_t = int[10][20][30];
    std::cout << rank<array_t>::value << std::endl;

    RemoveConst<const int>::type nonConstInt = 3;
    nonConstInt = 4;
    std::cout << nonConstInt << std::endl;

    RemoveConst2<const int>::type nonConstInt2 = 9;
    nonConstInt2 = 6;
    std::cout << nonConstInt2 << std::endl;

    std::cout << f(1) << std::endl;
    std::cout << f(2.7) << std::endl;

    // IsOneOf
    std::cout << IsOneOf<int, float, std::string, bool, void>::value << std::endl;
    std::cout << IsOneOf<void, float, std::string, bool, void>::value << std::endl;

    // Unevaluated operands
    aFuncRetType1 d = 3;
    aFuncRetType2 e = false;
    std::cout << d << " is type: " << typeid(aFuncRetType1).name() << ", " << e 
        << " is type: " << typeid(aFuncRetType2).name() << std::endl;

    std::cout << "Is string copy assignable: " << IsCopyAssignable<std::string>::type::value << std::endl;
    std::cout << "Is const string copyable assignable: " << IsCopyAssignable<const std::string>::type::value 
        << std::endl;

    // void_t
    std::cout << "Do you have a type member: " << HasTypeMember<IsCopyAssignable<int>>::value << std::endl;
    std::cout << "Do you have a type member: " << HasTypeMember<std::string>::value << std::endl;

    // IsCopyAssignable2 with void_t
    std::cout << "Is string copy assignable: " << IsCopyAssignable2<std::string>::type::value << std::endl;
    std::cout << "Is const string copyable assignable: " << IsCopyAssignable2<const std::string>::type::value 
        << std::endl;

    return 0;
}
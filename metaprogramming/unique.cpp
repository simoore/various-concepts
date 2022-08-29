// https://www.mavensecurities.com/revisiting-interview-questions-at-maven/

#include <algorithm>
#include <iostream>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////
// Vector: A compile time list of zero or more integers.
///////////////////////////////////////////////////////////////////////////////

// This is just an empty struct that is used to store a list of ints in its template parameters.
// `int... I` is called a parameter pack. `int` is the type of objects that the pack can contain, and `I` is the
// name of the pack.
template <int... I> struct Vector {};

///////////////////////////////////////////////////////////////////////////////
// VectorConcat: Joins two Vectors together.
///////////////////////////////////////////////////////////////////////////////

// We declare the primary template without definition. The declaration indicates that this metafunction always takes 
// exactly two types, but the specialization should always be used since there is no definition.
template<typename T1, typename T2>
struct VectorConcat;

// In the specialization, we take in two vectors containing 0 or more ints. We simplate return a vector containing
// the two parameter packs of the input parameters.
template <int... Is, int... Js>
struct VectorConcat<Vector<Is...>, Vector<Js...>> {
    using type = Vector<Is..., Js...>;
};

// Another way to implement this is to use a constexpr function. You can inspect the return to see the type of the
// two lists added.
template<int... Is, int... Js>
constexpr auto operator+(Vector<Is...>, Vector<Js...>) {
    return Vector<Is..., Js...>{};
}

///////////////////////////////////////////////////////////////////////////////
// Uniq: Eliminates duplicate entries from a sorted list.
///////////////////////////////////////////////////////////////////////////////

// Primary template, we just declare it, expecting that it will be never used. The result should use a specialization.
// The primary tempalte does indicate that calls to this metafunction only has one parameter.
template <typename T>
struct Uniq;

// This specialization operates on vectors with two or more parameters. This is where the main action happens. We use
// standard conditional to see if the first two elements of the list are the same, if they are discard one and recall
// Uniq. If they are not, perform Uniq on the tail on the list and preappend the first element.
template <int I1, int I2, int... Is>
struct Uniq<Vector<I1, I2, Is...>> {
private:
    using TrueResult = Uniq<Vector<I1, Is...>>::type;
    using FalseResult = VectorConcat<Vector<I1>, typename Uniq<Vector<I2, Is...>>::type>::type;
    using FalseResult2 = decltype(Vector<I1>{} + typename Uniq<Vector<I2, Is...>>::type{});

public:
    using type = std::conditional_t<I1 == I2, TrueResult, FalseResult2>;
};

// If a list has one element, then just return the list.
template <int I>
struct Uniq<Vector<I>> {
    using type = Vector<I>;
};

// If the list has no elements, then just return the empty list.
template <>
struct Uniq<Vector<>> {
    using type = Vector<>;
};

// constexpr approach - primary template
template <typename T>
struct Uniq2;

template <int... Is>
struct Uniq2<Vector<Is...>> {
private:
    static constexpr size_t originalSize = sizeof...(Is);

    struct Pair {
        int values[originalSize];
        size_t size;
    };

    static constexpr auto createUniqueArray = []{
        Pair p{{Is...}, originalSize};
        p.size = std::unique(p.values, p.values + originalSize) - p.values;
        return p;
    };

    static constexpr Pair valuePair = createUniqueArray();

    static constexpr auto valuesToVector = []<size_t... Js>(std::index_sequence<Js...>) {
        return Vector<valuePair.values[Js]...>();
    };

    static constexpr auto sequence = std::make_index_sequence<valuePair.size>();

public:
    using type = decltype(valuesToVector(sequence));
};

// Do third version that attempts to re-create Uniq1 with constexpr functions.
// constexpr approach - primary template
template <typename T>
struct Uniq3;

template <int... Js>
struct Uniq3<Vector<Js...>> {
private:

    template <int I>
    static constexpr auto unique(Vector<I>) {
        return Vector<I>();
    }

    template <int I1, int I2, int... Is>
    static constexpr auto unique(Vector<I1, I2, Is...>) {
        if constexpr (I1 == I2) {
            return unique(Vector<I1, Is...>());
        } else {
            return Vector<I1>() + unique(Vector<I2, Is...>());
        }
    }

public:
    using type = decltype(unique(Vector<Js...>()));
};
///////////////////////////////////////////////////////////////////////////////
// PrintVector: Prints the contents of a Vector at runtime.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct PrintVector;

template <int I, int... Is>
struct PrintVector<Vector<I, Is...>> {
    static void execute() {
        std::cout << I << " ";
        PrintVector<Vector<Is...>>::execute();
    }
};

template <>
struct PrintVector<Vector<>> {
    static void execute() {
        std::cout << std::endl;
    }
};

template<typename T>
struct PrintVector2;

template <int... Is>
struct PrintVector2<Vector<Is...>> {
    static void execute() {
        auto l = [](int x){ std::cout << x << " "; };

        // With the comma operator used like this: exp1, exp2, exp3
        // Each expression is evaluated in order and the last exp is returned as a result. It is almost the same as
        // a semicolon operator except that expressions in a semi-colon operator are considered to be part of the
        // same statement.
        (l(Is) , ...);
        std::cout << "\n";
    }
};

template <int... Is>
constexpr void printVector3(Vector<Is...>) {
    auto l = [](int x){ std::cout << x << " "; };
    (l(Is) , ...);
    std::cout << "\n";
}

///////////////////////////////////////////////////////////////////////////////
// static_assert and main
///////////////////////////////////////////////////////////////////////////////

// This is the check to see if out implentation of the uniq function works.
static_assert(std::is_same_v<Uniq<Vector<1, 2, 2, 2, 3, 4, 4, 5>>::type, Vector<1, 2, 3, 4, 5>>);
static_assert(std::is_same_v<Uniq<Vector<>>::type, Vector<>>);
static_assert(std::is_same_v<Uniq<Vector<1>>::type, Vector<1>>);
static_assert(std::is_same_v<Uniq<Vector<1, 1>>::type, Vector<1>>);
static_assert(std::is_same_v<Uniq<Vector<1, 2>>::type, Vector<1, 2>>);

int main() {
    PrintVector<Vector<1, 2, 3>>::execute();
    PrintVector<Uniq<Vector<1, 1>>::type>::execute();
    PrintVector<Uniq<Vector<1, 2>>::type>::execute();

    PrintVector2<Vector<4, 112, 727>>::execute();

    printVector3(Vector<65, 113, 711>{});

    PrintVector<Uniq2<Vector<5, 8, 34>>::type>::execute();
    PrintVector<Uniq2<Vector<1, 2, 2, 2, 3, 4, 4, 5>>::type>::execute();

    PrintVector2<Uniq3<Vector<14, 86, 86, 130>>::type>::execute();
    PrintVector2<Uniq3<Vector<1, 2, 2, 2, 3, 4, 4, 5>>::type>::execute();
}

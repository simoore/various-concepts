#include <type_traits>

// This is just an empty struct that is used to store a list of ints in its template parameters.
// `int... I` is called a parameter pack. `int` is the type of objects that the pack can contain, and `I` is the
// name of the pack.
template <int... I> struct Vector {};

//
template <typename Vector>
struct uniq {



    using type =
};

template <int... I>
struct unique<Vector<I...>> {
    static auto constexpr uni
};


// This is the check to see if out implentation of the uniq function works.
static_assert(std::is_same_v<uniq<Vector<1, 2, 2, 2, 3, 4, 4, 5>>::type, Vector<1, 2, 3, 4, 5>>);


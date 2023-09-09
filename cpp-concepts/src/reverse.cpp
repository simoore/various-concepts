#include <iostream>
#include <type_traits>
#include <tuple>

template <typename T>
struct Reverse;

template <typename... T1s, typename... T2s>
constexpr auto operator+(std::tuple<T1s...>, std::tuple<T2s...>) {
    return std::tuple<T1s..., T2s...>{};
}

template <typename T>
struct Reverse<std::tuple<T>> {
    using type = std::tuple<T>;
};

template <typename T1, typename T2, typename... Ts>
struct Reverse<std::tuple<T1, T2, Ts...>> {
    using type = decltype(typename Reverse<std::tuple<T2, Ts...>>::type{} + std::tuple<T1>{}); 
};

static_assert(std::is_same_v<typename Reverse<std::tuple<int, bool, double>>::type, std::tuple<double, bool, int>>);

int main() {
    Reverse<std::tuple<bool, int, double>>::type val = {3.4, 14, true};
    std::cout << std::get<0>(val) << " " << std::get<1>(val) << " " << std::get<2>(val) << std::endl;
};

cmake_minimum_required(VERSION 3.26)
project(CppConcepts LANGUAGES C CXX)

include(FetchContent)

# set(BOOST_INCLUDE_LIBRARIES thread asio)
# set(BOOST_ENABLE_CMAKE ON)
# FetchContent_Declare(
#     Boost
#     GIT_REPOSITORY https://github.com/boostorg/boost.git
#     GIT_TAG boost-1.83.0
#     GIT_SHALLOW TRUE)
# FetchContent_MakeAvailable(Boost)

add_compile_options(-std=c++23 -Og -Wall)

add_executable(AtomicApp src/atomic.cpp)
add_executable(BinarySearchTreeApp src/binary_search_tree.cpp)
add_executable(BuySellApp src/buy_sell.cpp)
add_executable(ConceptsApp src/concepts.cpp)
add_executable(ComparisonOperatorApp src/comparison_operator.cpp)
add_executable(ConcurrencyConditionVariablesApp src/concurrency_condition_variables.cpp)
add_executable(ConcurrencyLocksApp src/concurrency_locks.cpp)
add_executable(ConcurrencyThreadsApp src/concurrency_threads.cpp)
add_executable(CoroutineApp src/coroutines.cpp)
target_compile_options(CoroutineApp PRIVATE -fcoroutines)
target_include_directories(ConcurrencyThreadsApp PRIVATE include)
add_executable(ExamplesMetaprogrammingApp src/examples_metaprogramming.cpp)
add_executable(ExceptionsApp src/exceptions.cpp)
add_executable(FoldApp src/fold.cpp)
add_executable(ForeachApp src/foreach.cpp)
target_include_directories(ForeachApp PRIVATE include)
add_executable(LValuesRValuesApp src/lvalues_rvalues.cpp)
add_executable(MoveSemanticsApp src/move_semantics.cpp)
add_executable(NewOperator src/new_operator.cpp)
add_executable(Parallel1 src/parallel1.cpp)
add_executable(Parallel2 src/parallel2.cpp)
add_executable(ParallelFind src/parallel_find.cpp)
add_executable(ParallelSTL src/parallel_stl.cpp)
#target_link_libraries(Parallel2 PRIVATE Boost::thread Boost::asio)
add_executable(RangesApp src/ranges.cpp)
add_executable(ReverseApp src/reverse.cpp)
add_executable(QuickSortApp src/quick_sort.cpp)
add_executable(UniqueApp src/unique.cpp)
add_executable(VectorApp src/vector.cpp)


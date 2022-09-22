#include <iostream>
#include <memory>

struct NoisyObject {
    NoisyObject() {
        mId = mCount;
        ++mCount;
        std::cout << "Created object " << mId << std::endl;
    }

    ~NoisyObject() {
        std::cout << "Deleted object " << mId << std::endl;
    }

    static inline int mCount{0};
    int mId;
};

int main() {

    // The basic example, create two objects, delete them.
    NoisyObject *o1 = new NoisyObject;
    NoisyObject *o2 = new NoisyObject;
    delete o1;
    delete o2;

    // This calls 5 constructors, then 5 destructors.
    std::cout << std::endl << ">> New and delete expression for arrays" << std::endl;
    NoisyObject *arr1 = new NoisyObject[5];
    delete[] arr1;

    // If you want allocate memory without calling the constructor. This should have a random ID. Note that 
    // the constructor isn't called but the destructor is.
    std::cout << std::endl << ">> New operator and delete expression" << std::endl;
    NoisyObject *o3 = reinterpret_cast<NoisyObject *>(::operator new(sizeof(NoisyObject)));
    *o3 = NoisyObject();
    delete o3;

    // Here neither constructor or destructor are used. You'd have to manually call the destructor.
    std::cout << std::endl << ">> New and delete operator" << std::endl;
    NoisyObject *o4 = reinterpret_cast<NoisyObject *>(::operator new(sizeof(NoisyObject)));
    *o4 = NoisyObject();
    o4->~NoisyObject();
    ::operator delete(reinterpret_cast<void *>(o4));

    // ::operator new() vs ::operator new[](), basically you can assign different allocators to the two operators.
    // https://stackoverflow.com/questions/24603142/why-is-operator-new-necessary-when-operator-new-is-enough


    // Let's examine how a vector might manage memory.
    // https://stackoverflow.com/questions/63276697/how-to-implement-vectorclear-study-purpose
    // It saids:
    // 1. Allocate memory with operator new
    // 2. Call constructor with in-place constructor
    // 3. Destroy object explicity
    // 4. de-allocate with operator delete
    std::cout << std::endl << ">> Vector push_back and clear" << std::endl;
    NoisyObject *vec = reinterpret_cast<NoisyObject *>(::operator new(sizeof(NoisyObject) * 3));
    new (vec) NoisyObject();
    new (vec + 1) NoisyObject();
    new (vec + 2) NoisyObject();
    vec->~NoisyObject();
    (vec + 1)->~NoisyObject();
    (vec + 2)->~NoisyObject();
    ::operator delete(reinterpret_cast<void *>(vec));

    // There are some helper functions in STL to make this easier like std::uninitialized_default_construct and 
    // std::destroy which constructor and destroy objects already allocated memory.
    // This calls the constructor on an array of uninitialized memory.
    std::cout << std::endl << ">> Using std::uninitialized_default_construct" << std::endl;
    NoisyObject *vec2 = reinterpret_cast<NoisyObject *>(::operator new(sizeof(NoisyObject) * 3));
    std::uninitialized_default_construct(vec2, vec2 + 3);
    std::destroy(vec2, vec2 + 3);
    ::operator delete(reinterpret_cast<void *>(vec2));

    // This is an example from https://en.cppreference.com/w/cpp/memory/uninitialized_copy of creating memory for
    // a list of strings, copying literal strings into them, and then destroying the strings and freeing the memory.
    std::cout << std::endl << ">> Using std::uninitialized_copy" << std::endl;
    const char *v[] = {"This", "is", "an", "example"};
    auto sz = std::size(v);
    if(void *pbuf = std::aligned_alloc(alignof(std::string), sizeof(std::string) * sz)) {
        try {
            auto first = static_cast<std::string*>(pbuf);
            auto last = std::uninitialized_copy(std::begin(v), std::end(v), first);
            for (auto it = first; it != last; ++it)
                std::cout << *it << '_';
            std::cout << '\n';
            std::destroy(first, last);
        }
        catch(...) {}
        std::free(pbuf);
    }

    // The next step is understanding std::allocator.
}
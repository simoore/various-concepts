#include <algorithm>
#include <iostream>

template <typename T>
class Vector {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using iterator = T*;
    using const_iterator = const T*;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS: CONSTRUCTORS, DESTRUCTORS, ASSIGNMENTS
    ///////////////////////////////////////////////////////////////////////////

    // Creates a new vector with a set of repeated initial values.
    // numElements  Number of elements to initialize in the vector.
    // init         The initial value of each element.
    Vector(int numElements, const T& init = T()) {
        mCapacity = mSize = std::max(numElements, 0);
        mData = new T[mCapacity];
        std::fill(mData, mData + mSize, init);
    }

    // Constructor given an initializer list.
    Vector(std::initializer_list<T> init) {
        mSize = mCapacity = init.size();
        mData = new T[mCapacity];
        std::copy(init.begin(), init.end(), mData);
    }

    // Copy constructor.
    Vector(const Vector<T>& other) {
        mCapacity = other.mCapacity;
        mSize = other.mSize;
        mData = new T[mCapacity];
        std::copy(other.mData, other.mData + mSize, mData);
    }

    // Clears data on destruction.
    ~Vector() {
        delete[] mData;
    }

    // Copy assignment
    Vector<T>& operator=(const Vector<T>& other) {
        if (this != &other) {
            delete[] mData;
            mCapacity = other.mCapacity;
            mSize = other.mSize;
            mData = new T[mCapacity];
            std::copy(other.mData, other.mData + mSize, mData);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    int size() const {
        return size;
    }

    bool empty() const {
        return size == 0;
    }

    int capacity() const {
        return capacity;
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS: ACCESSORS
    ///////////////////////////////////////////////////////////////////////////

    // front(), back(), operator[]() all assume that the vector has data at the given location.

    T& front() {
        return mData[0];
    }

    const T& front() const {
        return mData[0];
    }

    T& back() {
        return mData[mSize - 1];
    }

    const T& back() const {
        return mData[mSize - 1];
    }

    T& operator[](int index) {
        return mData[index];
    }

    const T& operator[](int index) const {
        return mData[index];
    }

    T& at(int index) {
        if (index < 0 || index >= mSize) {
            throw std::out_of_range("Vector index out of range");
        }
        return mData[index];
    }

    const T& at(int index) const {
        if (index < 0 || index >= mSize) {
            throw std::out_of_range("Vector index out of range");
        }
        return mData[index];
    }

    
    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS: MUTATORS
    ///////////////////////////////////////////////////////////////////////////

    void push_back(const T& value) {
        resize(mSize + 1);
        mData[mSize - 1] = value;
    }

    void pop_back() {
        if (mSize > 0) {
            --mSize;
        }
    }

    void clear() {
        mSize = 0;
    }

    // Resizes the vector, and fills any additional values with init.
    void resize(int newSize, const T& init = T()) {
        if (newSize > mCapacity) {
            T* oldData = mData;
            mCapacity = std::max(newSize, 2 * mCapacity);
            mData = new T[mCapacity];
            std::copy(oldData, oldData + mSize, mData);
            delete[] oldData;
        }
        if (newSize > mSize) {
            std::fill(mData + mSize, mData + newSize, init);
        }
        mSize = std::max(newSize, 0);
    }

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS: ITERATORS
    ///////////////////////////////////////////////////////////////////////////

    iterator begin() {
        return mData;
    }

    const_iterator begin() const {
        return mData;
    }

    iterator end() {
        return mData + mSize;
    }

    const_iterator end() const {
        return mData + mSize;
    }

private:

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    T* mData;       // The array in memory.
    int mCapacity;  // The amount of allocated data in array.
    int mSize;      // The amount of valid data in the array.
};

int main() {
    Vector<int> vec = {1, 56, 34};
    for (auto v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}
// Move Semantics in C++
// https://www.youtube.com/watch?v=ehMg6zvXuMY
//
// std::move and the Move Assignment Operator in C++
// https://www.youtube.com/watch?v=OWNeCTd7yQE
//
// In converting a variable to an rvalue reference using std::move, you are essentially making the variable as 
// temporary and its resources can be taken by a move contructor or move assignment operator.

#include <iostream>
#include <cstring>
#include <cstdint>

class String {
public:
    String() = default;
    
    String(const char *string) {
        printf("Created!\n");
        mSize = std::strlen(string);
        mData = new char[mSize];
        memcpy(mData, string, mSize);
    }

    // The copy constructor (as identified by taking in a const lvalue reference of another instance of this object) 
    // just copies data out of the other object. The const enforces that the original object is unchanged.
    String(const String &other) {
        printf("Copied!\n");
        mSize = other.mSize;
        mData = new char[mSize];
        memcpy(mData, other.mData, mSize);
    }

    // An rvalue reference must call this function, but within the function and expression using just `other` is an
    // lvalue. It is convention that the move contructor perform a shallow copy and then null the input `other` 
    // object to perform the move.
    String(String &&other) noexcept {
        printf("Moved!\n");
        mSize = other.mSize;
        mData = other.mData;
        other.mSize = 0;
        other.mData = nullptr;
    }

    // The move assignment needs to delete existing data then perform the operations of the move constructor.
    // Also don't try do a move it `other` is this because you delete the data than try and copy the data you just
    // deleted and its messy.
    String& operator=(String &&other) noexcept {
        printf("Move assignment\n");
        if (this == &other) {
            return *this;
        }
        delete[] mData;
        mSize = other.mSize;
        mData = other.mData;
        other.mSize = 0;
        other.mData = nullptr;
        return *this;
    }

    ~String() {
        printf("Destroyed\n");
        delete[] mData;
    }

    void print() {
        printf("%.*s\n", mSize, mData);
    }

private:
    char *mData{nullptr};
    uint32_t mSize{0};
};

class Entity {
public:
    Entity(const String &name): mName(name) {}

    // This function is called using a rvalue reference (temporary value), however, an expression inside the function
    // that is just the parameter `name` is an lvalue because it is no longer temporary in the context of the function.
    // Thus the std::move is required to cast it back to an rvalue reference to call the move constructor.
    Entity(String &&name): mName(std::move(name)) {}

    void printName() {
        mName.print();
    }

private:
    String mName;
};

int main() {
    //Entity entity("Steve");
    //entity.printName();
    
    String apple = "apple";
    String dest;

    std::cout << "Apple: ";
    apple.print();
    std::cout << "Dest: ";
    dest.print();

    // move assignment
    dest = std::move(apple);

    std::cout << "Apple: ";
    apple.print();
    std::cout << "Dest: ";
    dest.print();
    
}
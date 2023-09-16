#pragma once

#include <memory>
#include <mutex>
#include <thread>

template <typename T>
class SequentialQueue {
public:

    // We create a dummy node with no real data to help with concurrent access to both pop and push at the same
    // time.
    SequentialQueue(): mHead(new Node), mTail(mHead.get()) {}

    Node *getTail(void) {
        std::lock_guard<std::mutex> lg(mTailMutex);
        return mTail;
    }

    void push(T value) {

        // change current dummy nodes data value
        auto newData = std::make_shared<T>(std::move(value));
        auto p = std::make_unique<Node>();
        const Node *newTail = p.get();
        {
            std::lock_guard<std::mutex> tlg(mTailMutex);
            mTail->data = newData;
            mTail->next = std::move(p);
            mTail = newTail;
        }
    }

    std::shared_ptr<T> pop(void) {

        // If we just lock the head mutex here (since we are only modifying mHead) we get a race condition with the 
        // push function. We need to lock both, but that constrains concurrent access to this data structure. Only 
        // one thread can use the data structure at a time.

        std::lock_guard<std::mutex> lg(mHeadMutex);
        if (head.get() == getTail()) {
            return std::shared_ptr<T>();
        }
        const auto res = std::make_shared<T>(std::move(mHead->data));
        const auto oldHead = std::move(mHead);
        mHead = std::move(oldHead->next);
        return res;  
    }

private:

    struct Node {
        T mData;
        std::unique_ptr<Node> mNext;

        Node(T data) : mData(std::move(data)) {}
    };

    std::unique_ptr<Node> mHead;
    Node *mTail;
    std::mutex mHeadMutex;
    std::mutex mTailMutex;

};
// TODO: STL iterator classes
// TODO: STL allocator classes
// TODO: Use smart pointers
// TODO: rebalance the tree
// TODO: catch2 unit tests.
// TODO: enable_chared_from_this, make_shared

#include <algorithm>
#include <functional>
#include <iostream>

template <typename T, typename Compare = std::less<T>>
class BinarySearchTree {
private:

    ///////////////////////////////////////////////////////////////////////////
    // HELPER CLASSES
    ///////////////////////////////////////////////////////////////////////////

    // A node in the tree. I
    struct Node {
        T mValue;
        Node *mLeft;
        Node *mRight;
        Node *mParent;

        Node(const T &value, Node *parent = nullptr, Node *left = nullptr, Node *right = nullptr):
            mValue(value), mLeft(left), mRight(right), mParent(parent) {}
    };

public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    // Iterator template class for binary search tree.
    template <typename U>
    class Iter {
    public:
        Iter(Node *node): mNode(node) { }

        // Dereference returns the value stored in the node.
        U &operator*() const {
            return mNode->mValue;
        }

        // Member returns a pointer to the value stored in the node.
        U *operator->() const {
            return &(mNode->mValue);
        }

        // Iterators are equal if they point to the same node.
        //
        // TODO: Replace with <=> operator.
        bool operator==(const Iter<U> &other) {
            return mNode == other.mNode;
        }

        bool operator!=(const Iter<U> &other) {
            return mNode != other.mNode;
        }

        // Prefix++. We change this iterator and just return it. The next node is found using in-order traversal.
        Iter<U> &operator++() {
            mNode = inorderSuccessor(mNode);
            return *this;
        }

        // Postfix++. Because we modify this iterator, but we want to return the original iterator, we make a copy
        // of this iterator and return by value. The next node is found using in-order traversal.
        Iter<U> operator++(int) {
            Iter<U> result = Iter<U>(mNode);
            mNode = inorderSuccessor(mNode);
            return result;
        }

    private:
        Node *mNode{nullptr};

    }; // class Iter

    using ThisType = BinarySearchTree<T, Compare>;
    using iterator = Iter<T>;
    using const_iterator = Iter<const T>;

    ///////////////////////////////////////////////////////////////////////////
    // CONSTRUCTORS, DESTRUCTORS, ASSIGNMENTS
    ///////////////////////////////////////////////////////////////////////////

    BinarySearchTree() = default;

    // Copy constructor.
    BinarySearchTree(const ThisType& other) {
        mRoot = copyTree(other.mRoot);
        mNumNodes = other.mNumNodes;
        mFirst = findFirst(mRoot);
    }

    // Move constructor.
    BinarySearchTree(ThisType &&other) {
        mRoot = other.mRoot;
        mNumNodes = other.mNumNodes;
        mFirst = other.mFirst;
        other.mRoot = nullptr;
        other.mNumNodes = 0;
        other.mFirst = nullptr;
    }

    BinarySearchTree(std::initializer_list<T> init): BinarySearchTree() {
        for (auto &p : init) {
            insert(p);
        }
    }

    ~BinarySearchTree() {
        freeAllNodes(mRoot);
    }

    // Copy assignment. Copy the tree and the number of nodes from `other`. Search for first in-order node.
    ThisType &operator=(const ThisType &other) {
        if (this != &other) {
            freeAllNodes(mRoot);
            mRoot = copyTree(other.mRoot);
            mNumNodes = other.mNumNodes;
            mFirst = findFirst(mRoot);
        }
        return *this;
    } 

    // Move assignment, free existing data, perform shallow copy, null `other`. Do nothing if `other` is this.
    ThisType &operator=(ThisType &&other) {
        if (this != &other) {
            freeAllNodes(mRoot);
            mRoot = other.mRoot;
            mNumNodes = other.mNumNodes;
            mFirst = other.mFirst;
            other.mRoot = nullptr;
            other.mNumNodes = 0;
            other.mFirst = nullptr;
        }
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////
    // ACCESSORS
    ///////////////////////////////////////////////////////////////////////////

    // The number of nodes in the tree. 
    int size() const {
        return mNumNodes;
    }

    // True if no nodes exist in the tree.
    bool empty() const {
        return size() == 0;
    }

    // Counts the number of occurances of `value` in the BST. Note that BST are constrained to only store
    // 1 copy of a value. 
    int count(const T &value) const {
        if (search(mRoot, value) == nullptr) {
            return 0;
        }
        return 1;
    }

    iterator find(const T &value) {
        return iterator(search(mRoot, value));
    }

    ///////////////////////////////////////////////////////////////////////////
    // MUTATORS
    ///////////////////////////////////////////////////////////////////////////

    // Inserts `value ` into the BST and returns the iterator pointing to this value. Since all values in a BST must 
    // be unique, no insertion will take place if the value is already in the BST.
    iterator insert(const T &value) {
        Node *node = add(mRoot, value);
        if (!mRoot) {
            mRoot = node;
        }
        return iterator(node);
    }

    // Remove node with `value` from the BST. If it doesn't exist in the BST, nothing is done.
    void erase(const T &value) {
        Node *node = search(mRoot, value);
        if (node != nullptr) {
            node = removeNode(node);
            --mNumNodes;
        }
    }

    // Deallocates all nodes in the tree.
    void clear() {
        freeAllNodes(mRoot);
        mRoot = nullptr;
        mFirst = nullptr;
        mNumNodes = 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // ITERATORS
    ///////////////////////////////////////////////////////////////////////////

    iterator begin() {
        return iterator{mFirst};
    }

    const_iterator cbegin() const {
        return const_iterator{mFirst};
    }

    iterator end() {
        return iterator{nullptr};
    }

    const_iterator cend() const {
        return const_iterator{nullptr};
    }

private:

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    Node *search(Node *node, const T &value) {
        if (node == nullptr) {
            return node;
        } else if (mCmp(value, node->mValue)) {
            return search(node->mLeft, value);
        } else if (mCmp(node->mValue, value)) {
            return search(node->mRight, value);
        } else {
            return node;
        }
    }

    // Add `value` to BST whose root is `node`. `node` can be nullptr to signify an empty tree. `parent` is the parent
    // node if a new node is created.
    Node *add(Node *node, const T &value, Node *parent = nullptr) {
        std::cout << "add(): " << node << " " << value << " " << parent << std::endl;
        if (node == nullptr) {
            std::cout << "Adding new node: " << value << std::endl;
            node = new Node(value, parent);
            ++mNumNodes;

            // We compare to the mFirst to see if the new value is first in an in-order transversal.
            if (mFirst == nullptr || mCmp(node->mValue, mFirst->mValue)) {
                mFirst = node;
            }
            return node;
        } else if (mCmp(value, node->mValue)) {
            Node *newNode = add(node->mLeft, value, node);
            if (!node->mLeft) {
                node->mLeft = newNode;
            }
            return newNode;
        } else if (mCmp(node->mValue, value)) {
            Node *newNode = add(node->mRight, value, node);
            if (!node->mRight) {
                node->mRight = newNode;
            }
            return newNode;
        } else {
            // Ignore duplicate insertions and just retun the node with the value.
            return node;
        }
    }

    // Removes `node` from the BST and returns the node to take its place in the tree.
    Node *removeNode(Node *node) {
        if (!node) {
            // Not a tree, do nothing.
            return nullptr;
        }

        if (mFirst == node) {
            mFirst = inOrderSuccessor(node);
        } 
        
        if (!node->mLeft && !node->mRight) {
            // Node is a leaf, just delete the node. Nothing is take its location.
            delete node;
            return nullptr;
        } else if (!node->mLeft || !node->mRight) {
            // If the node to be removed has one child, it child node replaces it in the tree.
            Node *successor = (node->mLeft) ? node->mLeft : node->mRight;
            successor->mParent = node->mParent;
            delete node;
            return successor;
        } else {
            // If the node to be removed has two children, choose the in-order successor to replace it. Rather than
            // delete the node, just copy the new data into it a delete the successor. The in-order successor should
            // be a leaf.
            Node *successor = inorderSuccessor(node);
            node->mValue = successor->mValue;
            removeNode(successor);
            return node;
        }
    }

    // Frees child tree, than frees itself.
    static inline void freeAllNodes(Node* node) {
        if (node) {
            freeAllNodes(node->mLeft);
            freeAllNodes(node->mRight);
            delete node;
        }
    }

    // Copies the tree starting at `node`.
    static inline Node *copyTree(Node *node, Node *parent = nullptr) {
        if (node) {
            Node *result = new Node(node->mValue, parent);
            result->mLeft = copyTree(node->mLeft, result);
            result->mRight = copyTree(node->mRight, result);
            return result;
        }
        return nullptr;
    }

    // Find first node, this is the first node for in-order traversals. This is the left-most descendant.
    static inline Node *findFirst(Node *root) {
        auto *first = root;
        if (root) {
            while (first->mLChild) {
                first = first->mLChild;
            }
        }
        return first;
    }

    // In-order traversal visits the left child tree first, then itself, then the right child tree. The in-order
    // sucessor is the node that directly follows this node in an in-order travesal.
    static inline Node *inorderSuccessor(Node *node) {
        if (!node) {
            // This isn't a tree.
            return nullptr;
        } else if (node->mRight) {
            // If the right child exists, it is the left-most descendant from the right-child.
            node = node->mRight;
            while (node->mLeft) {
                node = node->mLeft;
            }
        } else {
            // If no child or only the left child exists, the inorder is an ancestor. Search back up the tree until
            // you either find the root (then that must be the next one), or you've come from a left-child, then the
            // node is next in in-order travesal.
            Node *prev = node;
            node = node->mParent;
            while (node && prev == node->mRight) {
                prev = node;
                node = node->mParent;
            }
        }
        return node;
    }

    ///////////////////////////////////////////////////////////////////////////
    // PRIVATE VARIABLES
    ///////////////////////////////////////////////////////////////////////////

    Node *mRoot{nullptr};
    Node *mFirst{nullptr};
    int mNumNodes{0};
    Compare mCmp;

}; // class BinaryTreeSearch

int main() {
    std::cout << "Binary Search Tree" << std::endl;

    BinarySearchTree<int> tree = {1, 3, 5, 3, 2};
    std::for_each(tree.begin(), tree.end(), [](int x){ std::cout << x << " ";});
    std::cout << std::endl;
}
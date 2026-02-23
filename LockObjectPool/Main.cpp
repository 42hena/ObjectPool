#include <iostream>

#include "LockStackObjectPool.h"
#include "LockQueueObjectPool.h"



class Knight
{
public:
    Knight() {
        std::cout << "cons\n";
    }
    ~Knight() {
        std::cout << "dest\n";
    }
    int a;
};


struct Test
{
    int a;
};

int main()
{
    //LockQueueNode<Test>* node = new LockQueueNode<Test>();
    LockStackNode<Test>* node = new LockStackNode<Test>();

    std::cout << "node address           : " << node << std::endl;
    std::cout << "&node->_data           : " << &node->_data << std::endl;
    std::cout << "&node->_next           : " << &node->_next << std::endl;

    Test* dataPtr = &node->_data;

    /*LockQueueNode<Test>* wrong =
        reinterpret_cast<LockQueueNode<Test>*>(dataPtr);

    std::cout << "reinterpret_cast result: " << wrong << std::endl;*/

    delete node;
}

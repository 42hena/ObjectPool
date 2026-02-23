#include <iostream>
#include "TLSBucketObjectPool.h"
#include "TLSObjectPool.h"



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


int main()
{
    Knight* arr[32];
    for (int i = 0; i < 32; ++i)
    {
        arr[i] = TLSObjectPool<Knight>::PopPool();
    }
}

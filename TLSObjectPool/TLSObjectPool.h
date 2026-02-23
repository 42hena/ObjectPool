#pragma once
#include <mutex>
#include <iostream>

template<typename T>
struct TLSNode
{
    T data;
    TLSNode* _next = nullptr;
    TLSNode* _nextBucket = nullptr;
};

template <typename T>
class TLSObjectPool;


enum eObjectPool
{
    eObjectPool_BucketCount = 10,
    eObjectPool_NodeCount = 256,
};


template <typename Type>
class TLSObjectPoolManager
{
public:
    TLSObjectPoolManager();
    ~TLSObjectPoolManager();

public:
    TLSObjectPoolManager(const TLSObjectPoolManager&)               = delete;
    TLSObjectPoolManager& operator=(const TLSObjectPoolManager&)    = delete;
    TLSObjectPoolManager(TLSObjectPoolManager&&)                    = delete;
    TLSObjectPoolManager& operator=(TLSObjectPoolManager&&)         = delete;

public:
    static TLSObjectPoolManager& Instance();

    static void PushToManager(TLSNode<Type>* block);
    static TLSNode<Type>* PopFromManager();
    
public:
    TLSNode<Type>*  _topBucket;
    int             _useBucketCount;
    int             _totalBucketCount;

    static thread_local TLSObjectPool<Type>* _tlsObjectPool;
    std::mutex _lock;

};

template <typename Type>
thread_local TLSObjectPool<Type>* TLSObjectPoolManager<Type>::_tlsObjectPool = nullptr;






template<typename Type>
inline TLSObjectPoolManager<Type>::TLSObjectPoolManager()
    : _topBucket(nullptr)
    , _useBucketCount(0)
    , _totalBucketCount(0)
{
    int bucketIndex;
    int nodeIndex;
    for (bucketIndex = 0 ; bucketIndex < eObjectPool_BucketCount; bucketIndex += 1)
    {
        //_totalBucketCount++;
        TLSNode<Type>* prevNode = nullptr;

        for (nodeIndex = 0; nodeIndex < eObjectPool_NodeCount; nodeIndex += 1)
        {
            TLSNode<Type>* newNode = nullptr;
            newNode = new TLSNode<Type>();
            newNode->_next = prevNode;
            prevNode = newNode;
        }

        prevNode->_nextBucket = _topBucket;
        _topBucket = prevNode;
        //newBucket->_bucketNext = _allocBucketList;
    }

    printf("this: %p\n"
        "_topBucket: %p\n"
        "_useBucketCount: %p\n"
        "_totalBucketCount:%p\n"
        "_tlsObjectPool:%p %p\n"
        "_lock:%p\n", this, &_topBucket, &_useBucketCount, &_totalBucketCount,
        &_tlsObjectPool, _tlsObjectPool, &_lock);
}

template<typename Type>
inline TLSObjectPoolManager<Type>::~TLSObjectPoolManager()
{
}

template<typename Type>
inline TLSObjectPoolManager<Type>& TLSObjectPoolManager<Type>::Instance()
{
    static TLSObjectPoolManager instance;

    return instance;
}

template<typename Type>
inline void TLSObjectPoolManager<Type>::PushToManager(TLSNode<Type>* block)
{
    TLSObjectPoolManager& ref = TLSObjectPoolManager::Instance();

    std::lock_guard<std::mutex> lock(ref._lock);

    block->_nextBucket = ref._topBucket;
    ref._topBucket = block;

    ref._useBucketCount -= 1;
}

template<typename Type>
inline TLSNode<Type>* TLSObjectPoolManager<Type>::PopFromManager()
{
    TLSObjectPoolManager& ref = TLSObjectPoolManager::Instance();

    std::lock_guard<std::mutex> lock(ref._lock);

    TLSNode<Type>* bucket = ref._topBucket;
    ref._topBucket = ref._topBucket->_nextBucket;
    return bucket;
}





template <typename Type>
class TLSObjectPool
{
public:
    TLSObjectPool();
    ~TLSObjectPool();

public:
    TLSObjectPool(const TLSObjectPool&)             = delete;
    TLSObjectPool& operator=(const TLSObjectPool&)  = delete;
    TLSObjectPool(TLSObjectPool&&)                  = delete;
    TLSObjectPool& operator=(TLSObjectPool&&)       = delete;

public:
    static void PushPool(Type* data);
    static Type* PopPool();
    
private:
    int _allocSize = 0;
    //int _freeSize = 0;

    TLSNode<Type>* _allocList = nullptr;// _top
    //TLSNodeV2<Type>* _freeList = nullptr;
};

template<typename Type>
inline TLSObjectPool<Type>::TLSObjectPool()
{
}

template<typename Type>
inline TLSObjectPool<Type>::~TLSObjectPool()
{
}

template<typename Type>
inline void TLSObjectPool<Type>::PushPool(Type* data)
{
    if (TLSObjectPoolManager<Type>::_tlsObjectPool == nullptr)
    {
        TLSObjectPoolManager<Type>::_tlsObjectPool = new TLSObjectPool<Type>();
    }

    TLSObjectPool<Type>& ref = *TLSObjectPoolManager<Type>::_tlsObjectPool;

    TLSNode<Type>* newTop = reinterpret_cast<TLSNode<Type>*>(data);

    TLSNode<Type>*& top = ref._allocList;

    if ((++(ref._allocSize)) <= eObjectPool_NodeCount)
    {
        newTop->_next = top;
        top = newTop;
    }
    else
    {
        TLSObjectPoolManager<Type>::PushToManager(top);
        top = newTop;
        ref._allocSize = 1;
    }
}

template<typename Type>
inline Type* TLSObjectPool<Type>::PopPool()
{
    if (TLSObjectPoolManager<Type>::_tlsObjectPool == nullptr)
    {
        TLSObjectPoolManager<Type>::_tlsObjectPool = new TLSObjectPool<Type>();
    }

    TLSObjectPool<Type>& ref = *TLSObjectPoolManager<Type>::_tlsObjectPool;


    TLSNode<Type>* data = ref._allocList;
    if (data != nullptr)
    {
        ref._allocList = ref._allocList->_next;
        ref._allocSize -= 1;
        return reinterpret_cast<Type*>(data);
    }
    else
    {
        ref._allocList = TLSObjectPoolManager<Type>::PopFromManager();
        ref._allocSize += (eObjectPool_NodeCount - 1);
    }
    data = ref._allocList;
    ref._allocList = ref._allocList->_next;

    return reinterpret_cast<Type*>(data);
}

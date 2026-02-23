#pragma once

#include <mutex>

template<typename Type>
struct TLSBucketNode
{
    Type data;
    TLSBucketNode* _next = nullptr;
    TLSBucketNode* _nextBucket = nullptr;
};

template<typename Type>
struct Bucket
{
    Bucket*                 _bucketNext;
    TLSBucketNode<Type>*    _top;
    int                     _allocSize;
    int                     _freeSize;
};

enum eBucketObjectPool
{
    eBucketObjectPool_BucketCount = 10,
    eBucketObjectPool_NodeCount = 256,
};

template <typename Type>
class TLSBucketObjectPool;

template <typename Type>
class TLSBucketObjectPoolManager
{
public:
    TLSBucketObjectPoolManager();
    ~TLSBucketObjectPoolManager();

public:
    TLSBucketObjectPoolManager(const TLSBucketObjectPoolManager&)               = delete;
    TLSBucketObjectPoolManager& operator=(const TLSBucketObjectPoolManager&)    = delete;
    TLSBucketObjectPoolManager(TLSBucketObjectPoolManager&&)                    = delete;
    TLSBucketObjectPoolManager& operator=(TLSBucketObjectPoolManager&&)         = delete;

public:
    static TLSBucketObjectPoolManager& Instance();
    
    static void PushToManager(TLSBucketNode<Type>* block);
    static TLSBucketNode<Type>* PopFromManager();

public:
    Bucket<Type>* _allocBucketList;
    Bucket<Type>* _freeBucketList;
    
    int _useBucketCount;
    int _totalBucketCount;

    std::mutex _lock;
    static thread_local TLSBucketObjectPool<Type>* _tlsObjectPool;
};






template<typename Type>
inline TLSBucketObjectPoolManager<Type>::TLSBucketObjectPoolManager()
    : _allocBucketList(nullptr)
    , _freeBucketList(nullptr)
    , _useBucketCount(0)
    , _totalBucketCount(0)
{
    int bucketIndex;
    int nodeIndex;
    for (int bucketIndex = 0; bucketIndex < eBucketObjectPool_BucketCount; bucketIndex += 1)
    {
        Bucket<Type>* newBucket = new Bucket<Type>();

        _totalBucketCount += 1;

        TLSBucketNode<Type>* newNode = nullptr;
        for (nodeIndex = 0; nodeIndex < eBucketObjectPool_NodeCount; nodeIndex += 1)
        {
            newNode = new TLSBucketNode<Type>();

            newNode->_next = newBucket->_top;
            newBucket->_top = newNode;
        }
        newBucket->_bucketNext = _allocBucketList;
        _allocBucketList = newBucket;
    }
}

template<typename Type>
inline TLSBucketObjectPoolManager<Type>::~TLSBucketObjectPoolManager()
{
    
}

template<typename Type>
inline TLSBucketObjectPoolManager<Type>& TLSBucketObjectPoolManager<Type>::Instance()
{
    static TLSBucketObjectPoolManager _instance;

    return _instance;
}

template<typename Type>
inline void TLSBucketObjectPoolManager<Type>::PushToManager(TLSBucketNode<Type>* block)
{
    TLSBucketObjectPoolManager& ref = TLSBucketObjectPoolManager::Instance();

    // lock
    std::lock_guard<std::mutex> lock(ref._lock);

    Bucket<Type>* bucket = ref._freeBucketList;
    bucket->_top = block;
    
    ref._freeBucketList = ref._freeBucketList->_bucketNext;
    bucket->_bucketNext = ref._allocBucketList;
    
    ref._allocBucketList = bucket;
    ref._useBucketCount -= 1;
}

template<typename Type>
inline TLSBucketNode<Type>* TLSBucketObjectPoolManager<Type>::PopFromManager()
{
    TLSBucketObjectPoolManager& ref = TLSBucketObjectPoolManager::Instance();
    
    // 락 가드
    std::lock_guard<std::mutex> lock(ref._lock);

    // 껍데기 가져오기.
    Bucket<Type>* bucket = ref._allocBucketList;
    ref._allocBucketList = ref._allocBucketList->_bucketNext;

    // 껍데기에 node 뭉치 채우기
    TLSBucketNode<Type>* top = bucket->_top;
    ref._useBucketCount += 1;

    bucket->_bucketNext = ref._freeBucketList;
    ref._freeBucketList = bucket;

    return top;
}






template <typename Type>
class TLSBucketObjectPool
{
public:
    TLSBucketObjectPool();
    ~TLSBucketObjectPool();

public:
    TLSBucketObjectPool(const TLSBucketObjectPool&)             = delete;
    TLSBucketObjectPool& operator=(const TLSBucketObjectPool&)  = delete;
    TLSBucketObjectPool(TLSBucketObjectPool&&)                  = delete;
    TLSBucketObjectPool& operator=(TLSBucketObjectPool&&)       = delete;

public:
    static TLSBucketObjectPool& GetInstance();

    static void PushPool(Type* data);
    static Type* PopPool();

private:
    int _totalSize = 0;
    int _freeSize = 0;

    TLSBucketNode<Type>* _allocList = nullptr;
    TLSBucketNode<Type>* _freeList = nullptr;
};

template<typename Type>
thread_local TLSBucketObjectPool<Type>*
TLSBucketObjectPoolManager<Type>::_tlsObjectPool = nullptr;






template<typename Type>
inline TLSBucketObjectPool<Type>::TLSBucketObjectPool()
{
}

template<typename Type>
inline TLSBucketObjectPool<Type>::~TLSBucketObjectPool()
{
}

template<typename Type>
inline TLSBucketObjectPool<Type>& TLSBucketObjectPool<Type>::GetInstance()
{
    static TLSBucketObjectPool instance;

    return instance;
}

template<typename Type>
inline void TLSBucketObjectPool<Type>::PushPool(Type* data)
{
    if (TLSBucketObjectPoolManager<Type>::_tlsObjectPool == nullptr)
    {
        TLSBucketObjectPoolManager<Type>::_tlsObjectPool = new TLSBucketObjectPool<Type>();
    }

    TLSBucketObjectPool<Type>& ref = *TLSBucketObjectPoolManager<Type>::_tlsObjectPool;

    TLSBucketNode<Type>* newTop = reinterpret_cast<TLSBucketNode<Type>*>(data);
    if ((++(ref._freeSize)) > eBucketObjectPool_NodeCount)
    {
        TLSBucketObjectPoolManager<Type>::PushToManager(newTop);
        ref._freeList = newTop;
        ref._freeSize = 1;
    }
    else
    {
        newTop->_next = ref._freeList;
        ref._freeList = newTop;
    }
}

template<typename Type>
inline Type* TLSBucketObjectPool<Type>::PopPool()
{
    if (TLSBucketObjectPoolManager<Type>::_tlsObjectPool == nullptr)
    {
        TLSBucketObjectPoolManager<Type>::_tlsObjectPool = new TLSBucketObjectPool<Type>();
    }

    TLSBucketObjectPool<Type>& ref = *TLSBucketObjectPoolManager<Type>::_tlsObjectPool;
    if (ref._freeSize != 0)
    {
        TLSBucketNode<Type>* data = ref._freeList;
        ref._freeList = ref._freeList->_next;
        ref._freeSize -= 1;

        return reinterpret_cast<Type*>(data);
    }

    TLSBucketNode<Type>* data = ref._allocList;
    if (data == nullptr)
    {
        ref._allocList = TLSBucketObjectPoolManager<Type>::PopFromManager();
        data = ref._allocList;
    }
    ref._allocList = ref._allocList->_next;

    return reinterpret_cast<Type*>(data);
}

#pragma once

template <typename Type>
struct LockQueueNode
{
    Type            _data;
    LockQueueNode*  _next;
};

enum eLockQueueObjectPool
{
    eLockQueueObjectPool_ObjectCount = 256
};


template <typename Type>
class LockQueueObjectPool
{
public:
#pragma region SPECIAL_MEMBER_FUNCTION 
    LockQueueObjectPool(int objectCount = eLockQueueObjectPool_ObjectCount, bool placementNew = false);
    ~LockQueueObjectPool();

public:
    LockQueueObjectPool(const LockQueueObjectPool&) = delete;
    LockQueueObjectPool(LockQueueObjectPool&&) = delete;
    LockQueueObjectPool& operator=(const LockQueueObjectPool&) = delete;
    LockQueueObjectPool& operator=(LockQueueObjectPool&&) = delete;
#pragma endregion

public:
    void Push(Type* dataAddress);
    Type* Pop();
    
public:
    __forceinline int GetUseCount() { return _totalCount - _freeCount; }
    __forceinline int GetFreeCount() { return _freeCount; }
    __forceinline int GetTotalCount() { return _totalCount; }

private:
    void CreateNodes();
    void DestroyNodes();

#pragma region ¸â¹ö º¯¼ö
private:
    SRWLOCK         _lock;
    LockQueueNode<Type>* _front;
    LockQueueNode<Type>* _rear;
    int             _freeCount;
    int             _totalCount;
    bool            _placementNew;
#pragma endregion
};






template<typename Type>
inline LockQueueObjectPool<Type>::LockQueueObjectPool(int objectCount, bool placementNew)
    :_front(nullptr)
    , _rear(nullptr)
    , _lock()
    , _placementNew(placementNew)
    , _freeCount(objectCount)
    , _totalCount(objectCount)
{
    InitializeSRWLock(&_lock);
    CreateNodes();
}

template<typename Type>
inline LockQueueObjectPool<Type>::~LockQueueObjectPool()
{
    DestroyNodes();
}

template<typename Type>
inline void LockQueueObjectPool<Type>::Push(Type* dataAddress)
{
    if (_placementNew == true)
    {
        dataAddress->~Type();
    }

    LockQueueNode<Type>* pNode = reinterpret_cast<LockQueueNode<Type>*>(dataAddress);

    pNode->_next = nullptr; // Debug

    SRWLockGuard lock(_lock);
    
    _rear->_next = pNode;
    _rear = pNode;

    _freeCount += 1;
}

template<typename Type>
inline Type* LockQueueObjectPool<Type>::Pop()
{
    SRWLockGuard lock(_lock);

    LockQueueNode<Type>* pNode = _front->_next;
    if (GetFreeCount() == 0)
    {
        LockQueueNode<Type>* newNode;
        if (_placementNew == false)
        {
            newNode = new LockQueueNode<Type>();
        }
        else
        {
            newNode = static_cast<LockQueueNode<Type>*>(malloc(sizeof(LockQueueNode<Type>)));
            new (reinterpret_cast<Type*>(newNode)) Type();
        }

        _totalCount += 1;

        return reinterpret_cast<Type*>(newNode);
    }
    _front->_next = pNode->_next;
    pNode->_next = nullptr;

    _freeCount -= 1;

    return reinterpret_cast<Type*>(pNode);
}

template<typename Type>
inline void LockQueueObjectPool<Type>::CreateNodes()
{
    if (_placementNew == false)
    {
        LockQueueNode<Type>* newNode = new LockQueueNode<Type>();
        newNode->_next = nullptr;
        _front = _rear = newNode;
        for (int i = 0; i < _totalCount; ++i)
        {
            newNode = new LockQueueNode<Type>();
            _rear->_next = newNode;
            _rear = newNode;
        }
        _rear->_next = nullptr;
    }
    else
    {
        LockQueueNode<Type>* newNode = static_cast<LockQueueNode<Type>*>(malloc(sizeof(LockQueueNode<Type>)));
        newNode->_next = nullptr;
        _front = _rear = newNode;
        for (int i = 0; i < _totalCount; ++i)
        {
            newNode = static_cast<LockQueueNode<Type>*>(malloc(sizeof(LockQueueNode<Type>)));
            _rear->_next = newNode;
            _rear = newNode;
        }
    }
}

template<typename Type>
inline void LockQueueObjectPool<Type>::DestroyNodes()
{
    while (_front)
    {
        LockQueueNode<Type>* curNode = _front;
        _front = _front->_next;
        if (_placementNew)
        {
            free(curNode);
        }
        else
        {
            delete curNode;
        }
    }
}

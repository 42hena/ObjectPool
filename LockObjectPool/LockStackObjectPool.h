#pragma once
#include <mutex>
#include <Windows.h>

#include "LockGuard.h"

template <typename Type>
struct LockStackNode
{
    Type            _data;
    LockStackNode*  _next;
};

enum eLockStackObjectPool
{
    eLockStackObjectPool_ObjectCount = 256
};

template <typename Type>
class LockStackObjectPool
{
#pragma region 특수 멤버 함수
public:
    LockStackObjectPool(int objectCount = eLockStackObjectPool_ObjectCount, bool placementNew = false);
    ~LockStackObjectPool();

public:
    LockStackObjectPool(const LockStackObjectPool&) = delete;
    LockStackObjectPool& operator=(const LockStackObjectPool&) = delete;
    LockStackObjectPool(LockStackObjectPool&&) = delete;
    LockStackObjectPool& operator=(LockStackObjectPool&&) = delete;
#pragma endregion

public:
    void Push(Type* dataAddress);
    Type* Pop();

public:
    __forceinline int GetUseCount()     { return _totalCount - _freeCount; }
    __forceinline int GetFreeCount()    { return _freeCount; }
    __forceinline int GetTotalCount()   { return _totalCount; }

private:
    void CreateNodes();
    void DestroyNodes();

#pragma region 멤버 변수
private:
    LockStackNode<Type>* _top;
    SRWLOCK             _lock;
    int                 _freeCount;
    int                 _totalCount;
    bool                _placementNew;
#pragma endregion
};






template <typename Type>
LockStackObjectPool<Type>::LockStackObjectPool<Type>(int objectCount, bool placementNew)
    : _top(nullptr)
    , _placementNew(placementNew)
    , _freeCount(objectCount)
    , _totalCount(objectCount)
{
    InitializeSRWLock(&_lock);
    CreateNodes();
}

template<typename Type>
inline LockStackObjectPool<Type>::~LockStackObjectPool()
{
    DestroyNodes();
}

template<typename Type>
inline void LockStackObjectPool<Type>::Push(Type* dataAddress)
{
    // placementNew였으면 소멸자 호출
    if (_placementNew == true)
    {
        dataAddress->~Type();
    }

    LockStackNode<Type>* pNode = reinterpret_cast<LockStackNode<Type>*>(dataAddress);

    SRWLockGuard lock(_lock);

    pNode->_next = _top;
    _top = pNode;
    _freeCount += 1;
}

template<typename Type>
inline Type* LockStackObjectPool<Type>::Pop()
{
    LockStackNode<Type>* pNode;
    LockStackNode<Type>* newNode;
    
    SRWLockGuard lock(_lock);

    pNode = _top;
    if (pNode == nullptr)
    {
        if (_placementNew == false)
        {
            newNode = new LockStackNode<Type>();
        }
        else
        {
            newNode = static_cast<LockStackNode<Type>*>(malloc(sizeof(LockStackNode<Type>)));
            new (reinterpret_cast<Type*>(newNode)) Type();
        }
        ++_totalCount;
        return reinterpret_cast<Type*>(newNode);
    }

    _freeCount -= 1;
    _top = _top->_next;

    if (_placementNew == true)
    {
        new (reinterpret_cast<Type*>(pNode)) Type();
    }

    return reinterpret_cast<Type*>(pNode);
}

template<typename Type>
inline void LockStackObjectPool<Type>::CreateNodes()
{
    if (_placementNew == false)
    {
        for (int i = 0; i < _totalCount; ++i)
        {
            LockStackNode<Type>* newNode = new LockStackNode<Type>();
            newNode->_next = _top;
            _top = newNode;
        }
    }
    else
    {
        for (int i = 0; i < _totalCount; ++i)
        {
            LockStackNode<Type>* newNode = static_cast<LockStackNode<Type>*>(malloc(sizeof(LockStackNode<Type>)));
            newNode->_next = _top;
            _top = newNode;
        }
    }
}

template<typename Type>
inline void LockStackObjectPool<Type>::DestroyNodes()
{
    while (_top)
    {
        LockStackNode<Type>* curNode = _top;
        _top = _top->_next;
        if (_placementNew == false)
        {
            delete curNode;
        }
        else
        {
            free(curNode);
        }
    }
}

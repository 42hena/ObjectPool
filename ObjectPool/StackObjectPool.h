#pragma once

enum eStackObjectPool
{
    eStackObjectPool_DefaultObjectCount = 256
};

template <typename Type>
struct StackNode
{
    Type        data;
    StackNode*  next;
};

template <typename Type>
class StackObjectPool
{
#pragma region Æ¯¼ö ¸â¹ö ÇÔ¼ö
public:
    StackObjectPool(int objectCount = eStackObjectPool_DefaultObjectCount, bool placementNew = false);
    ~StackObjectPool();

public:
    StackObjectPool(const StackObjectPool&)             = delete;
    StackObjectPool& operator=(const StackObjectPool&)  = delete;
    StackObjectPool(StackObjectPool&&)                  = delete;
    StackObjectPool& operator=(StackObjectPool&&)       = delete;
#pragma endregion

public:
    __forceinline int GetUseCount() { return _totalCount - _freeCount; }
    __forceinline int GetFreeCount() { return _freeCount; }
    __forceinline int GetTotalCount() { return _totalCount; }

public:
    void Push(Type* memoryAddress);
    Type* Pop();

private:
    void InitObjectPool();
    void CreateNodes();
    void DestroyNodes();
    

#pragma region ¸â¹ö º¯¼ö
private:
    StackNode<Type>*    _topNode;
    int                 _freeCount;
    int                 _totalCount;
    bool                _placementNew;
#pragma endregion
};






template<typename Type>
inline StackObjectPool<Type>::StackObjectPool(int objectCount, bool placementNew)
    : _topNode(nullptr)
    , _placementNew(placementNew)
    , _freeCount(objectCount)
    , _totalCount(objectCount)
{
    InitObjectPool();
}

template<typename Type>
inline StackObjectPool<Type>::~StackObjectPool()
{
    DestroyNodes();
}

template<typename Type>
inline void StackObjectPool<Type>::Push(Type* memoryAddress)
{
    // ¼Ò¸êÀÚ È£Ãâ
    if (_placementNew == true)
    {
        memoryAddress->~Type();
    }

    StackNode<Type>* nodeAddress = reinterpret_cast<StackNode<Type>*>(memoryAddress);
    nodeAddress->next = _topNode;
    _topNode = nodeAddress;

    _freeCount += 1;
}

template<typename Type>
inline Type* StackObjectPool<Type>::Pop()
{
    StackNode<Type>* pNode;
    StackNode<Type>* newNode;

    pNode = _topNode;
    if (pNode == nullptr)
    {
        if (_placementNew == false)
        {
            newNode = new StackNode<Type>();
        }
        else
        {
            newNode = static_cast<StackNode<Type>*>(malloc(sizeof(StackNode<Type>)));
            new (reinterpret_cast<Type*>(newNode)) Type();
        }
        _totalCount += 1;
        return reinterpret_cast<Type*>(newNode);
    }
    _topNode = _topNode->next;
    
    _freeCount -= 1;

    if (_placementNew == true)
    {
        new (&pNode->data) Type();
    }

    return reinterpret_cast<Type*>(pNode);
}

template<typename Type>
inline void StackObjectPool<Type>::InitObjectPool()
{
    CreateNodes();
}

template<typename Type>
inline void StackObjectPool<Type>::CreateNodes()
{
    if (_placementNew == false)
    {
        for (int i = 0; i < _totalCount; ++i)
        {
            StackNode<Type>* newNode = new StackNode<Type>();
            newNode->next = _topNode;
            _topNode = newNode;
        }
    }
    else
    {
        for (int i = 0; i < _totalCount; ++i)
        {
            StackNode<Type>* newNode = static_cast<StackNode<Type>*>(malloc(sizeof(StackNode<Type>)));
            newNode->next = _topNode;
            _topNode = newNode;
        }
    }
}

template<typename Type>
inline void StackObjectPool<Type>::DestroyNodes()
{
    while (_topNode)
    {
        StackNode<Type>* curNode = _topNode;
        _topNode = _topNode->next;
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

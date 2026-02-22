#pragma once

enum eQueueObjectPool
{
    eQueueObjectPool_DefaultObjectCount = 5
};

template <typename Type>
struct QueueNode
{
    Type        data;
    QueueNode*  next;
};

template <typename Type>
class QueueObjectPool
{
public:
#pragma region Æ¯¼ö ¸â¹ö ÇÔ¼ö
    QueueObjectPool(int objectCount = eQueueObjectPool_DefaultObjectCount, bool placementNew = false);
    ~QueueObjectPool();

public:
    QueueObjectPool(const QueueObjectPool&)             = delete;
    QueueObjectPool(QueueObjectPool&&)                  = delete;
    QueueObjectPool& operator=(const QueueObjectPool&)  = delete;
    QueueObjectPool& operator=(QueueObjectPool&&)       = delete;
#pragma endregion

public:
    void Push(Type* memoryAddress);
    Type* Pop();
    
public:
    int             GetUseCount() { return _totalCount - _freeCount; }
    int             GetFreeCount() { return _freeCount; }
    int             GetTotalCount() { return _totalCount; }

private:
    void            CreateNodes();
    void            DestroyNodes();

#pragma region ¸â¹ö º¯¼ö
private:
    QueueNode<Type>* _front;
    QueueNode<Type>* _rear;
    int             _freeCount;
    int             _totalCount;
    bool            _placementNew;
#pragma endregion
};






template<typename Type>
inline QueueObjectPool<Type>::QueueObjectPool(int objectCount, bool placementNew)
    :_front(nullptr),
    _rear(nullptr),
    _placementNew(placementNew),
    _freeCount(objectCount),
    _totalCount(objectCount)
{
    CreateNodes();
}

template<typename Type>
inline QueueObjectPool<Type>::~QueueObjectPool()
{
    DestroyNodes();
}

template<typename Type>
inline void QueueObjectPool<Type>::Push(Type* memoryAddress)
{
    // ¼Ò¸êÀÚ È£Ãâ
    if (_placementNew == true)
    {
        memoryAddress->~Type();
    }

    QueueNode<Type>* node = reinterpret_cast<QueueNode<Type>*>(memoryAddress);

    if (node->next != nullptr)
    {
        __debugbreak();
        node->next = nullptr;
    }
    _rear->next = node;
    _rear = node;

    _freeCount += 1;
}

template<typename Type>
inline Type* QueueObjectPool<Type>::Pop()
{
    QueueNode<Type>* node = _front->next;
    if (GetFreeCount() == 0)
    {
        QueueNode<Type>* newNode;
        if (_placementNew == false)
        {
            newNode = new QueueNode<Type>();
            newNode->next = nullptr;
        }
        else
        {
            newNode = static_cast<QueueNode<Type>*>(malloc(sizeof(QueueNode<Type>)));
            new (reinterpret_cast<Type*>(newNode)) Type();
            newNode->next = nullptr;
        }

        ++_totalCount;
        return reinterpret_cast<Type*>(newNode);
    }

    _front->next = node->next;
    node->next = nullptr;
    --_freeCount;
    if (_freeCount == 0)
    {
        _rear = _front;
    }

    if (_placementNew == true)
    {
        new (reinterpret_cast<Type*>(node)) Type();
    }

    return reinterpret_cast<Type*>(node);
}

template<typename Type>
inline void QueueObjectPool<Type>::CreateNodes()
{
    if (_placementNew == false)
    {
        QueueNode<Type>* newNode = new QueueNode<Type>();
        newNode->next = nullptr;
        _front = _rear = newNode;
        for (int i = 0; i < _totalCount; ++i)
        {
            newNode = new QueueNode<Type>();
            newNode->next = nullptr;
            _rear->next = newNode;
            _rear = newNode;
        }
        _rear->next = nullptr;
    }
    else
    {
        QueueNode<Type>* newNode = static_cast<QueueNode<Type>*>(malloc(sizeof(QueueNode<Type>)));
        newNode->next = nullptr;
        _front = _rear = newNode;
        for (int i = 0; i < _totalCount; ++i)
        {
            newNode = static_cast<QueueNode<Type>*>(malloc(sizeof(QueueNode<Type>)));
            newNode->next = nullptr;
            _rear->next = newNode;
            _rear = newNode;
        }
    }
}

template<typename Type>
inline void QueueObjectPool<Type>::DestroyNodes()
{
    while (_front)
    {
        QueueNode<Type>* curNode = _front;
        _front = _front->next;
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
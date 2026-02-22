#include <iostream>
#include "Profiler.h"
#include "StackObjectPool.h"
#include "QueueObjectPool.h"

class AKnight
{
public:
    AKnight() { std::cout << "AKnight cons\n"; }
    ~AKnight() { std::cout << "AKnight dest\n"; }
    int GetA() { return a; }
    int Getb() { return b; }
    int a;
    int b;
};

class BKnight
{
public:
    BKnight() { std::cout << "BKnight cons\n"; }
    ~BKnight() { std::cout << "BKnight dest\n"; }
    int GetA() { return a; }
    int Getb() { return b; }
    long long a;
    long long b;
};

#define NUM 256
struct DataInfo
{
    char buf[8];
};

StackObjectPool<DataInfo> sop(NUM, false);
QueueObjectPool<DataInfo> qop(NUM, false);

int global_prev = 100;
int global_loop = 1000000;


void NewDeleteTest()
{
    DataInfo* arr[NUM];

    for (int w = 0; w < global_prev; ++w)
    {
        for (int i = 0; i < NUM; ++i)
        {
            arr[i] = new DataInfo;
        }

        for (int i = 0; i < NUM; ++i)
        {
            delete arr[i];
        }
    }
    std::cout << "New Delete Start\n";
    for (int w = 0; w < global_loop; ++w)
    {
        {
            ProfileMeasure n("New");
            for (int i = 0; i < NUM; ++i)
            {
                arr[i] = new DataInfo;
            }
        }

        {
            ProfileMeasure del("Delete");
            for (int i = 0; i < NUM; ++i)
            {
                delete arr[i];
            }
        }
    }
}

void StackObjectPoolTest()
{
    DataInfo* arr[NUM];

    for (int w = 0; w < global_prev; ++w)
    {
        for (int i = 0; i < NUM; ++i)
        {
            arr[i] = sop.Pop();
        }

        for (int i = 0; i < NUM; ++i)
        {
            sop.Push(arr[i]);
        }
    }
    std::cout << "Stack Start\n";
    for (int w = 0; w < global_loop; ++w)
    {
        {
            ProfileMeasure SPop("StackPop");
            for (int i = 0; i < NUM; ++i)
            {
                arr[i] = sop.Pop();
            }
        }

        {
            ProfileMeasure SPush("StackPush");
            for (int i = 0; i < NUM; ++i)
            {
                sop.Push(arr[i]);
            }
        }
    }
}

void QueueObjectPoolTest()
{
    DataInfo* arr[NUM*2];
    for (int w = 0; w < global_prev; ++w)
    {
        for (int i = 0; i < NUM; ++i)
        {
            arr[i] = qop.Pop();
        }

        for (int i = 0; i < NUM; ++i)
        {
            qop.Push(arr[i]);
        }
    }
    std::cout << "Queue Start\n";
    for (int w = 0; w < global_loop; ++w)
    {
        {
            ProfileMeasure QPush("QueuePop");
            for (int i = 0; i < NUM; ++i)
            {
                arr[i] = qop.Pop();
            }
        }

        {
            ProfileMeasure QPop("QueuePush");
            for (int i = 0; i < NUM; ++i)
            {
                qop.Push(arr[i]);
            }
        }
    }
}

int main()
{
    NewDeleteTest();
    StackObjectPoolTest();
    QueueObjectPoolTest();

    ProfilerUtil::ProfileDataOutText();
}

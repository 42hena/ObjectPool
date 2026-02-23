#pragma once

#include <mutex>
#include <Windows.h>

class CriticalSectionGuard
{
public:
    CriticalSectionGuard(CRITICAL_SECTION* lock);
    ~CriticalSectionGuard();

public:
    CriticalSectionGuard()                                          = delete;

    CriticalSectionGuard(const CriticalSectionGuard&)               = delete;
    CriticalSectionGuard& operator=(const CriticalSectionGuard&)    = delete;
    CriticalSectionGuard(CriticalSectionGuard&&)                    = delete;
    CriticalSectionGuard& operator=(CriticalSectionGuard&&)         = delete;

private:
    CRITICAL_SECTION* _lock;
};






class SRWLockGuard
{
public:
    SRWLockGuard(SRWLOCK* lock);
    ~SRWLockGuard();

public:
    SRWLockGuard()                                  = delete;

    SRWLockGuard(const SRWLockGuard&)               = delete;
    SRWLockGuard& operator=(const SRWLockGuard&)    = delete;
    SRWLockGuard(SRWLockGuard&&)                    = delete;
    SRWLockGuard& operator=(SRWLockGuard&&)         = delete;

private:
    SRWLOCK* _lock;
};






class MutexGuard
{
public:
    MutexGuard(std::mutex* lock);
    ~MutexGuard();

public:
    MutexGuard()                                = delete;
    MutexGuard(const MutexGuard&)               = delete;
    MutexGuard& operator=(const MutexGuard&)    = delete;
    MutexGuard(MutexGuard&&)                    = delete;
    MutexGuard& operator=(MutexGuard&&)         = delete;

private:
    std::mutex* _lock;
};






class RecursiveMutexGuard
{
public:
    RecursiveMutexGuard(std::recursive_mutex* lock);
    ~RecursiveMutexGuard();
    
public:
    RecursiveMutexGuard()                                       = delete;

    RecursiveMutexGuard(const RecursiveMutexGuard&)             = delete;
    RecursiveMutexGuard& operator=(const RecursiveMutexGuard&)  = delete;
    RecursiveMutexGuard(RecursiveMutexGuard&&)                  = delete;
    RecursiveMutexGuard& operator=(RecursiveMutexGuard&&)       = delete;

private:
    std::recursive_mutex* _lock;
};

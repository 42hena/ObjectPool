#include "LockGuard.h"

CriticalSectionGuard::CriticalSectionGuard(CRITICAL_SECTION* lock)
    : _lock(lock)
{
    EnterCriticalSection(_lock);
}

CriticalSectionGuard::~CriticalSectionGuard()
{
    LeaveCriticalSection(_lock);
}






SRWLockGuard::SRWLockGuard(SRWLOCK* lock)
    : _lock(lock)
{
    AcquireSRWLockExclusive(_lock);
}

SRWLockGuard::~SRWLockGuard()
{
    ReleaseSRWLockExclusive(_lock);
}






MutexGuard::MutexGuard(std::mutex* lock)
{
    _lock->lock();
}

MutexGuard::~MutexGuard()
{
    _lock->unlock();
}






RecursiveMutexGuard::RecursiveMutexGuard(std::recursive_mutex* lock)
    : _lock(lock)
{
    _lock->lock();
}

RecursiveMutexGuard::~RecursiveMutexGuard()
{
    _lock->unlock();
}

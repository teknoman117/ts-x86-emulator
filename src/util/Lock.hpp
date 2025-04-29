#ifndef LOCK_HPP
#define LOCK_HPP

#include <uvw/thread.h>
#include <concepts>

template<typename T>
concept LockableRef = requires(T a) {
    a.lock();
    a.unlock();
};

template<typename T>
concept LockablePtr = requires(T a) {
    a->lock();
    a->unlock();
};

template <typename T>
concept Lockable = LockableRef<T> || LockablePtr<T>;

template<Lockable L>
class Lock final {
    L& m;
public:
    Lock() = delete;
    ~Lock() { if constexpr (LockablePtr<L>) { m->unlock(); } else { m.unlock(); } }
    Lock(L& m_) : m{m_} { if constexpr (LockablePtr<L>) { m->lock(); } else { m.lock(); } }
    Lock(const Lock&) = delete;
    Lock(Lock&& l) : m{l.m} {}

    Lock& operator=(const Lock&) = delete;
    Lock& operator=(Lock&&) = delete;
};

#endif
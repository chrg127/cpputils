#pragma once

#include <mutex>

template <
    typename T,
    typename M = std::mutex,
    template <typename...> typename WL = std::unique_lock,
    template <typename...> typename RL = std::unique_lock
>
class MutexGuarded {
    mutable M mut;
    T elem;
    auto lock() const { return RL<M>(mut); }
    auto lock()       { return WL<M>(mut); }

public:
    MutexGuarded() = default;
    explicit MutexGuarded(T in)
        : elem(std::move(in))
    { }

    auto read(auto &&fn) const
    {
        auto l = lock();
        return fn(elem);
    }

    auto write(auto &&fn)
    {
        auto l = lock();
        return fn(elem);
    }
};

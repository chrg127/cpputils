#include <array>
#include <initializer_list>

// A vector allocated in-place, same as std::inplace_vector.
template <typename T, unsigned N>
class InPlaceVector {
    static_assert(N > 0, "N must be > 0");

    std::array<T, N> p;
    T *c = p.data();

public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       =       value_type &;
    using const_reference = const value_type &;
    using pointer         =       value_type *;
    using const_pointer   = const value_type *;
    using iterator        =       value_type *;
    using const_iterator  = const value_type *;

    constexpr InPlaceVector() = default;

    constexpr explicit InPlaceVector(size_type count)
    {
        for (auto i = 0u; i < count; i++) {
            p[i] = T();
        }
    }

    constexpr InPlaceVector(size_type count, const T &value)
    {
        for (auto i = 0u; i < count; i++) {
            p[i] = value;
        }
    }

    constexpr InPlaceVector(std::initializer_list<T> init)
    {
        for (auto i = 0u; i < init.size(); i++) {
            p[i] = init[i];
        }
    }

    constexpr InPlaceVector(const InPlaceVector<T, N>  &other) { p = other.p; }
    constexpr InPlaceVector(      InPlaceVector<T, N> &&other) { p = std::move(other.p); }

    constexpr reference at(size_type pos)
    {
        if (pos >= N) {
            throw std::out_of_range("InPlaceVector::at");
        }
        return p[pos];
    }

    constexpr const_reference at(size_type pos) const
    {
        if (pos >= N) {
            throw std::out_of_range("InPlaceVector::at");
        }
        return p[pos];
    }

    constexpr       reference operator[](size_type pos)                { return p[pos]; }
    constexpr const_reference operator[](size_type pos) const          { return p[pos]; }
    constexpr       reference front()                                  { return p[0]; }
    constexpr const_reference front()                   const          { return p[0]; }
    constexpr       reference back()                                   { return c[-1]; }
    constexpr const_reference back()                    const          { return c[-1]; }
    constexpr              T *data()                          noexcept { return p; }
    constexpr        const T *data()                    const noexcept { return p; }
    constexpr       iterator begin()                          noexcept { return p.begin(); }
    constexpr const_iterator begin()                    const noexcept { return p.begin(); }
    constexpr const_iterator cbegin()                   const noexcept { return p.cbegin(); }
    constexpr       iterator end()                            noexcept { return p.end(); }
    constexpr const_iterator end()                      const noexcept { return p.end(); }
    constexpr const_iterator cend()                     const noexcept { return p.cend(); }
    constexpr           bool empty()                    const noexcept { return p.data() == c; }
    constexpr      size_type size()                     const noexcept { return c - p.data(); }
    static constexpr size_type max_size()               const noexcept { return N; }
    static constexpr size_type capacity()               const noexcept { return N; }

    constexpr void resize(size_type count, T value = T())
    {
        auto diff = count - size();
        for (auto *ptr = c; ptr < c + diff; ptr++)
            *ptr = value;
        c += diff;
    }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        new (c) T(args...);
        c++;
        return c[-1];
    }

    constexpr           void push_back(const T &value)                 { *c++ = value; }
    constexpr           void push_back(T &&value)                      { *c++ = std::move(value); }
    constexpr           void pop_back()                                { c--; }
    constexpr           void clear() noexcept                          { c = p; }
};


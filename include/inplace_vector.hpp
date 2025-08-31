#include <array>
#include <initializer_list>
#include <stdexcept>

// A vector allocated in-place, same as std::inplace_vector.
template <typename T, unsigned N>
class InPlaceVector {
    static_assert(N > 0, "N must be > 0");

    std::array<T, N> p;
    std::size_t c = 0;

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
        c = count;
    }

    constexpr InPlaceVector(size_type count, const T &value)
    {
        for (auto i = 0u; i < count; i++) {
            p[i] = value;
        }
        c = count;
    }

    constexpr InPlaceVector(std::initializer_list<T> init)
    {
        for (auto i = 0u; i < init.size(); i++) {
            p[i] = *(init.begin() + i);
        }
        c = init.size();
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
    constexpr       reference back()                                   { return p[c-1]; }
    constexpr const_reference back()                    const          { return p[c-1]; }
    constexpr              T *data()                          noexcept { return p; }
    constexpr        const T *data()                    const noexcept { return p; }
    constexpr       iterator begin()                          noexcept { return p.begin(); }
    constexpr const_iterator begin()                    const noexcept { return p.begin(); }
    constexpr const_iterator cbegin()                   const noexcept { return p.cbegin(); }
    constexpr       iterator end()                            noexcept { return p.begin() + c; }
    constexpr const_iterator end()                      const noexcept { return p.begin() + c; }
    constexpr const_iterator cend()                     const noexcept { return p.cbegin() + c; }
    constexpr           bool empty()                    const noexcept { return c == 0; }
    constexpr      size_type size()                     const noexcept { return c; }
    static constexpr size_type max_size() noexcept { return N; }
    static constexpr size_type capacity() noexcept { return N; }

    constexpr void resize(size_type count, const T &value = T())
    {
        auto diff = count - size();
        for (auto i = c; i < c + diff; i++)
            p[i] = value;
        c += diff;
    }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        new (&p[c]) T(args...);
        c++;
        return p[c-1];
    }

    constexpr           void push_back(const T &value)                 { p[c++] = value; }
    constexpr           void push_back(T &&value)                      { p[c++] = std::move(value); }
    constexpr           void pop_back()                                { c--; }
    constexpr           void clear() noexcept                          { c = 0; }
};


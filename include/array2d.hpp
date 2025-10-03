#pragma once

#include <cstddef>
#include <span>

/* Just like std::array, but in two dimensions. Doesn't implement iterators. */
template <typename T, std::size_t Width, std::size_t Height>
class Array2D {
    static_assert(Width != 0 && Height != 0, "Can't define an Array2D with 0 width or height");
    T arr[Width*Height];

public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = std::span<T>;
    using const_reference = std::span<const T>;

    constexpr       reference operator[](size_type pos)                { return std::span{ arr+pos*Width, Width }; }
    constexpr const_reference operator[](size_type pos) const          { return std::span{ arr+pos*Width, Width }; }
    constexpr       reference front()                                  { return operator[](0); }
    constexpr const_reference front()                   const          { return operator[](0); }
    constexpr       reference back()                                   { return operator[](Height-1); }
    constexpr const_reference back()                    const          { return operator[](Height-1); }
    constexpr              T *data()                          noexcept { return arr; }
    constexpr        const T *data()                    const noexcept { return arr; }
    [[nodiscard]]
    constexpr      bool empty()                         const noexcept { return Height == 0 || Width == 0; }
    constexpr size_type width()                         const noexcept { return Width; }
    constexpr size_type height()                        const noexcept { return Height; }
};


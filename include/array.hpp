#pragma once

#include <cstddef>
#include <span>
#include <memory>

namespace util {

/* An array with size determined at run-time. This size never grows. */
template <typename T>
class HeapArray {
    std::unique_ptr<T[]> p;
    std::size_t l = 0;

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

    constexpr HeapArray() = default;
    constexpr HeapArray(size_type count, const T &value = T())
        : p(std::make_unique<T[]>(count)), l(count)
    {
        for (auto i = 0u; i < l; i++)
            p[i] = value;
    }

    template <typename It>
    constexpr HeapArray(It first, It last)
        : p(std::make_unique<T[]>(last - first)), l(last - first)
    {
        for (auto i = 0u; i < l; i++)
            p[i] = *(first + i);
    }

    constexpr HeapArray(std::initializer_list<T> init)
        : HeapArray(init.begin(), init.end())
    { }

    constexpr       reference operator[](size_type pos)                { return p[pos]; }
    constexpr const_reference operator[](size_type pos) const          { return p[pos]; }
    constexpr       reference front()                                  { return p[0]; }
    constexpr const_reference front()                   const          { return p[0]; }
    constexpr       reference back()                                   { return p[l-1]; }
    constexpr const_reference back()                    const          { return p[l-1]; }
    constexpr              T *data()                          noexcept { return p.get(); }
    constexpr        const T *data()                    const noexcept { return p.get(); }
    constexpr        iterator begin()                         noexcept { return p.get(); }
    constexpr  const_iterator begin()                   const noexcept { return p.get(); }
    constexpr  const_iterator cbegin()                  const noexcept { return p.get(); }
    constexpr        iterator end()                           noexcept { return p.get()+l; }
    constexpr  const_iterator end()                     const noexcept { return p.get()+l; }
    constexpr  const_iterator cend()                    const noexcept { return p.get()+l; }
    [[nodiscard]]
    constexpr            bool empty()                   const noexcept { return l == 0; }
    constexpr       size_type size()                    const noexcept { return l; }
};

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

/* A 2D view over a 1D array. */
template <typename T>
class Span2D {
    T *d;
    std::size_t w, h, s;

public:
    constexpr Span2D() : d(nullptr), w(0), h(0), s(0) {}
    constexpr Span2D(T *data, std::size_t width, std::size_t height, std::size_t stride)
        : d(data), w(width), h(height), s(stride)
    { }

    constexpr Span2D(T *d, std::size_t w, std::size_t h) : Span2D(d, w, h, 0) { }
    template <typename R> constexpr Span2D(R &&r, std::size_t w, std::size_t h) : Span2D(r.data(), w, h) { }

    constexpr Span2D(const Span2D &) noexcept = default;
    constexpr Span2D& operator=(const Span2D &) noexcept = default;

    using element_type      = T;
    using value_type        = std::remove_cv_t<T>;
    using size_type         = std::size_t;
    using reference         = std::span<T>;

    constexpr reference front()                 const          { return this->operator[](0); }
    constexpr reference back()                  const          { return this->operator[](w-1); }
    constexpr reference operator[](size_type y) const          { return std::span{&d[y * (w + s)], w}; }
    constexpr T *       data()                  const          { return d; }
    constexpr size_type width()                 const noexcept { return w; }
    constexpr size_type height()                const noexcept { return h; }
    constexpr size_type stride()                const noexcept { return s; }
    [[nodiscard]] constexpr bool empty()        const noexcept { return w == 0 || h == 0; }

    constexpr Span2D<T> subspan(size_type x, size_type y, size_type width, size_type height) const
    {
        return Span2D(&d[y * (w + s) + x], width, height, s + (w - width));
    }
};

} // namespace util

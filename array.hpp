#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <array>
#include <span>
#include <memory>

namespace util {

template <typename T>
class HeapArray {
    std::unique_ptr<T[]> ptr;
    std::size_t len = 0;

public:
    using iterator = T *;

    HeapArray() = default;
    explicit HeapArray(std::size_t s)           { reset(s); }
          T & operator[](std::size_t pos)       { return ptr[pos]; }
    const T & operator[](std::size_t pos) const { return ptr[pos]; }
          T *data()       noexcept              { return ptr.get(); }
    const T *data() const noexcept              { return ptr.get(); }
          iterator begin()       noexcept       { return ptr.get(); }
    const iterator begin() const noexcept       { return ptr.get(); }
          iterator end()                        { return ptr.get() + len; }
    const iterator end()   const noexcept       { return ptr.get() + len; }
    [[nodiscard]] bool empty() const noexcept   { return len == 0; }
    std::size_t size() const noexcept           { return len; }

    void reset(std::size_t s)
    {
        ptr = std::make_unique<T[]>(s);
        len = s;
    }
};

template <typename T, std::size_t Width, std::size_t Height>
class Array2D {
    static_assert(Width != 0 && Height != 0, "Can't define an Array2D with 0 width or height");
    T arr[Width*Height];

public:
    constexpr std::span<T>       operator[](std::size_t pos)       { return std::span{ arr+pos*Width, Width }; }
    constexpr std::span<const T> operator[](std::size_t pos) const { return std::span{ arr+pos*Width, Width }; }

    constexpr T *data() { return arr; }
    constexpr const T *data() const { return arr; }
    constexpr std::size_t width()  const { return Width; }
    constexpr std::size_t height() const { return Height; }
};

template <typename T, unsigned N>
class StaticVector {
    T arr[N];
    T *cur = arr;

public:
    constexpr StaticVector() = default;

    constexpr       T & operator[](std::size_t pos)       { return arr[pos]; }
    constexpr const T & operator[](std::size_t pos) const { return arr[pos]; }

    constexpr       T *data()        noexcept { return arr; }
    constexpr const T *data()  const noexcept { return arr; }

    using iterator = T *;

    constexpr       iterator begin()       noexcept { return arr; }
    constexpr const iterator begin() const noexcept { return arr; }
    constexpr       iterator end()         noexcept { return cur; }
    constexpr const iterator end()   const noexcept { return cur; }

    [[nodiscard]] constexpr bool empty() const noexcept { return cur == arr; }
    constexpr std::size_t size() const noexcept { return cur - arr; }

    constexpr void clear() noexcept          { cur = arr; }
    constexpr void push_back(const T &value) { *cur++ = value; }
    constexpr void push_back(T &&value)      { *cur++ = std::move(value); }
    constexpr void pop_back()                { cur--; }

    template <typename... Args>
    constexpr T & emplace_back(Args&&... args)
    {
        T *ptr = new (cur) T(args...);
        cur++;
        return *ptr;
    }
};

/* A 2D view over a 1D array. */
template <typename T>
struct Span2D {
    T *d;
    std::size_t w, h, s;

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

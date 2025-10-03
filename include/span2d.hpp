#include <cstddef>

/* A 2D view over a 1D array. */
template <typename T>
class Span2D {
    T *d;
    std::size_t w, h, s;

public:
    using element_type      = T;
    using value_type        = std::remove_cv_t<T>;
    using size_type         = std::size_t;
    using reference         = std::span<T>;

    constexpr Span2D() : d(nullptr), w(0), h(0), s(0) {}
    constexpr Span2D(T *data, std::size_t width, std::size_t height, std::size_t stride)
        : d(data), w(width), h(height), s(stride)
    { }

    constexpr Span2D(T *d, std::size_t w, std::size_t h) : Span2D(d, w, h, 0) { }
    template <typename R> constexpr Span2D(R &&r, std::size_t w, std::size_t h) : Span2D(r.data(), w, h) { }

    constexpr Span2D(const Span2D &) noexcept = default;
    constexpr Span2D& operator=(const Span2D &) noexcept = default;

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


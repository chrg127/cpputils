/*
 * The missing string function from the standard library.
 *
 * This is a collection of common functions for working with strings. Unless
 * specified, all these functions work generically in terms of the string type.
 * This means you can use them for working with std::string, std::string_view,
 * custom string types, etc.
 */

#pragma once

#include <algorithm>
#include <cstring>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <limits>
#include "concepts.hpp"

namespace string {

namespace detail {

template <typename T>
std::from_chars_result _from_chars_double(const char *first, T &value)
{
    char *endptr;
    value = std::strtod(first, &endptr);
    std::from_chars_result res;
    res.ptr = endptr,
    res.ec = value == 0 && endptr == first ? static_cast<std::errc>(errno)
           : std::errc();
    return res;
}

} // namespace detail

/*
 * Functions for checking if a character is a space / alphanumeric / digit.
 * Replaces similar C functions but without anything locale specific.
 */
inline bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r'; }
inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

/*
 * Splits a string into multiple strings, using delim as the delimiter between
 * them. Each string is then output to the function fn.
 */
template <typename T = std::string>
inline void split(const T &s, char delim, auto &&fn)
{
    for (std::size_t i = 0, p = 0; i != s.size(); i = p+1) {
        p = s.find(delim, i);
        fn(s.substr(i, (p == s.npos ? s.size() : p) - i));
        if (p == s.npos)
            break;
    }
}

/*
 * Splits a string into multiple strings. Instead of using a callback like
 * the other split() function, it returns an std::vector instead.
 */
template <typename From = std::string, typename To = std::string>
inline std::vector<To> split(const From &s, char delim = ',')
{
    std::vector<To> res;
    split<From>(s, delim, [&] (const auto &substr) {
        res.emplace_back(substr);
    });
    return res;
}

/*
 * A helper function for split() for easily splitting an std::string_view into
 * multiple ones.
 */
inline std::vector<std::string_view> split_view(std::string_view s, char delim = ',')
{
    return split<std::string_view, std::string_view>(s, delim);
}

/*
 * Splits a single string in multiple lines, each long 'width' characters.
 * The function tries splitting between words, where a word is defined
 * as a string without spaces.
 */
template <typename From = std::string, typename To = std::string>
inline std::vector<To> split_lines(const From &s, std::size_t width)
{
    std::vector<To> result;
    auto it = s.begin();
    while (it != s.end()) {
        it = std::find_if_not(it, s.end(), is_space);
        auto start = it;
        it += std::min(size_t(s.end() - it), size_t(width));
        it = std::find_if(it, s.end(), is_space);
        result.emplace_back(start, it);
    }
    return result;
}

/*
 * Trims a string, i.e. removes space from start and end of the string.
 * It always makes a new copy.
 */
template <typename From = std::string, typename To = std::string>
inline To trim(const From &s)
{
    auto i = std::find_if_not(s.begin(),  s.end(),  is_space);
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    return {i, j};
}

/* A helper function for using trim() with std::string_view. */
inline std::string_view trim_view(std::string_view s) { return trim<std::string_view, std::string_view>(s); }

/* Just like trim(), but it will modify the input string. */
template <typename T = std::string>
inline void trim_in_place(T &s)
{
    auto j = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
    s.erase(j, s.end());
    auto i = std::find_if_not(s.begin(), s.end(), is_space);
    s.erase(s.begin(), i);
}

/*
 * Converts a string to a number. Returns an empty optional if the string can't
 * be converted to a number for any reason.
 */
template <Number T = int, typename TStr = std::string>
inline std::optional<T> to_number(const TStr &str, int base = 10)
{
    auto helper = [](const char *start, const char *end, unsigned base) -> std::optional<T> {
        T value = 0;
        std::from_chars_result res;
        if constexpr(std::is_floating_point_v<T>)
            // GCC version < 11 doesn't have std::from_chars<double>, and this version
            // is still installed in some modern distros (debian stable, WSL ubuntu)
            res = detail::_from_chars_double<T>(start, value);
        else
            res = std::from_chars(start, end, value, base);
        if (res.ec != std::errc() || res.ptr != end)
            return std::nullopt;
        return value;
    };
    if constexpr(std::is_same_v<std::decay_t<TStr>, char *>)
        return helper(str, str + std::strlen(str), base);
    else if constexpr(std::is_same_v<std::decay_t<TStr>, const char *>)
        return helper(str, str + std::strlen(str), base);
    else
        return helper(str.data(), str.data() + str.size(), base);
}

/* Converting a number to a string. The function never fails. */
template <typename TStr = std::string, Number T = int>
inline TStr from_number(const T &n, int base = 10)
{
    if constexpr(std::is_floating_point_v<T>) {
        constexpr auto maxbuf = std::numeric_limits<T>::digits + 1
                              + std::is_signed<T>::value;
        std::array<char, maxbuf> buf;
        auto res = std::to_chars(buf.data(), buf.data() + buf.size(), n);
        auto str = TStr(buf.begin(), res.ptr);
        return str;
    } else {
        constexpr auto maxbuf = std::numeric_limits<T>::digits10 + 1
                              + std::is_signed<T>::value;
        std::array<char, maxbuf> buf;
        auto res = std::to_chars(buf.data(), buf.data() + buf.size(), n, base);
        auto str = TStr(buf.begin(), res.ptr);
        return str;
    }
}

} // namespace string

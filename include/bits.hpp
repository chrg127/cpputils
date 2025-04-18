#pragma once

#include <concepts>
#include <limits>
#include "common.hpp"

namespace bits {

/* Returns a mask usable to mask off a given number of bits.
 * For example: 3 -> 0b11; 6 -> 0b111111 */
constexpr inline u64 bitmask(u8 nbits) { return (1UL << nbits) - 1UL; }

/* Functions for getting or setting bits. */
constexpr inline u64 getbit( u64 num, u8 bitno)           { return num >> bitno & 1; }
constexpr inline u64 getbits(u64 num, u8 bitno, u8 nbits) { return num >> bitno & bitmask(nbits); }
constexpr inline u64 setbit(u64 num, u8 bitno, u8 data)   { return (num & ~(1 << bitno)) | (data & 1) << bitno; }

constexpr inline u64 setbits(u64 num, u8 bitno, u8 nbits, u64 data)
{
    const u64 mask = bitmask(nbits);
    return (num & ~(mask << bitno)) | (data & mask) << bitno;
}

/* Reverse the bits of a number. */
constexpr inline u8 reverse(u8 n)
{
    n = (n & 0xF0) >> 4 |  (n & 0x0F) << 4;
    n = (n & 0xCC) >> 2 |  (n & 0x33) << 2;
    n = (n & 0xAA) >> 1 |  (n & 0x55) << 1;
    return n;
}

template <typename T>
inline T reverse2(T v)
{
    T r = v;
    int s = sizeof(v) * CHAR_BIT - 1;
    for (v >>= 1; v; v >>= 1) {
        r <<= 1;
        r |= v & 1;
        s--;
    }
    r <<= s;
    return r;
}

/*
 * A portable bit-field type. To be used in unions, when you want to access
 * both a whole number and parts of it. For example:
 *
 * union {
 *     u16 full
 *     BitField<u16, 1, 1> flag;
 *     BitField<u16, 2, 3> flag_3bits;
 * } data;
 *
 * The types must necessarily be the same in order to work.
 */
template <std::integral T, unsigned Bitno, unsigned Nbits = 1>
class BitField {
    T data;
public:
    BitField() = default;
    BitField(const BitField<T, Bitno, Nbits> &b)              { operator=(b); }
    BitField & operator=(const BitField<T, Bitno, Nbits> &b)  { data = setbits(data, Bitno, Nbits, u64(b)); return *this; }
    BitField & operator=(const u64 val)                       { data = setbits(data, Bitno, Nbits, val);    return *this; }
    operator u64() const                                      { return (data >> Bitno) & bitmask(Nbits); }
    template <typename U> BitField & operator+= (const U val) { *this = *this +  val; return *this; }
    template <typename U> BitField & operator-= (const U val) { *this = *this -  val; return *this; }
    template <typename U> BitField & operator*= (const U val) { *this = *this *  val; return *this; }
    template <typename U> BitField & operator/= (const U val) { *this = *this /  val; return *this; }
    template <typename U> BitField & operator&= (const U val) { *this = *this &  val; return *this; }
    template <typename U> BitField & operator|= (const U val) { *this = *this |  val; return *this; }
    template <typename U> BitField & operator^= (const U val) { *this = *this ^  val; return *this; }
    template <typename U> BitField & operator>>=(const U val) { *this = *this >> val; return *this; }
    template <typename U> BitField & operator<<=(const U val) { *this = *this << val; return *this; }
                          BitField & operator++()             { return *this = *this + 1; }
                          BitField & operator--()             { return *this = *this - 1; }
    constexpr unsigned bitno() const                          { return Bitno; }
    constexpr unsigned nbits() const { return Nbits; }
};

/*
 * A 16 bit number that can be accessed either through its full value and
 * through its low and high byte.
 */
union Word {
    u16 v;
    struct { u8 l, h; };
    Word() = default;
    explicit Word(u16 val) : v(val) {}
    Word & operator= (u16 val) { v  = val; return *this; }
    Word & operator&=(u64 val) { v &= val; return *this; }
    Word & operator|=(u64 val) { v |= val; return *this; }
};

// Returns largest power of 2 that is <= the given value.
template <typename T>
constexpr inline T largest_power_of_2_within(T n)
    requires std::is_unsigned_v<T>
{
    // Set all bits less significant than most significant bit that's set.
    // e.g. 0b00100111  ->  0b00111111
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    if constexpr(std::numeric_limits<T>::digits == 16) n |= (n >>  8);
    if constexpr(std::numeric_limits<T>::digits == 32) n |= (n >> 16);
    if constexpr(std::numeric_limits<T>::digits == 64) n |= (n >> 32);
    // Clear all set bits besides the highest one.
    return n - (n >> 1);
}

/* Literals for expressing byte units. */
namespace literals {
    inline constexpr std::size_t operator"" _KiB(unsigned long long bytes) { return bytes*1024; }
    inline constexpr std::size_t operator"" _MiB(unsigned long long bytes) { return bytes*1024*1024; }
    inline constexpr std::size_t operator"" _GiB(unsigned long long bytes) { return bytes*1024*1024*1024; }
    inline constexpr std::size_t operator"" _TiB(unsigned long long bytes) { return bytes*1024*1024*1024*1024; }

    inline constexpr std::size_t operator"" _KB(unsigned long long bytes) { return bytes*1000; }
    inline constexpr std::size_t operator"" _MB(unsigned long long bytes) { return bytes*1000*1000; }
    inline constexpr std::size_t operator"" _GB(unsigned long long bytes) { return bytes*1000*1000*1000; }
    inline constexpr std::size_t operator"" _TB(unsigned long long bytes) { return bytes*1000*1000*1000*1000; }

    inline constexpr std::size_t operator"" _Kib(unsigned long long bytes) { return bytes*1024/8; }
    inline constexpr std::size_t operator"" _Mib(unsigned long long bytes) { return bytes*1024*1024/8; }
    inline constexpr std::size_t operator"" _Gib(unsigned long long bytes) { return bytes*1024*1024*1024/8; }
    inline constexpr std::size_t operator"" _Tib(unsigned long long bytes) { return bytes*1024*1024*1024*1024/8; }
} // namespace literals

} // namespace bits

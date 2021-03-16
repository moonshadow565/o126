#ifndef O126_COMMON_HPP
#define O126_COMMON_HPP
#include <array>
#include <bit>
#include <cinttypes>

namespace o126 {
using byte_t = std::uint8_t;
using word_t = std::uint16_t;
using dword_t = std::uint32_t;

using sbyte_t = std::int8_t;
using sword_t = std::int16_t;
using sdword_t = std::int32_t;

struct BUS;
struct CPU;
struct PIC;
struct PIT;

template <typename T>
[[nodiscard]] static constexpr auto to_signed(T val) noexcept {
    return static_cast<std::make_signed_t<T>>(val);
}

constexpr auto word_unpack(word_t value) noexcept {
    struct Pair {
        byte_t lo;
        byte_t hi;
    };
    return Pair { static_cast<byte_t>(value), static_cast<byte_t>(value >> 8) };
}

constexpr auto word_pack(byte_t lo, byte_t hi) noexcept {
    return static_cast<word_t>(lo | (hi << 8));
}

struct FAR final {
    word_t disp = {};
    word_t seg = {};

    constexpr FAR() noexcept = default;

    constexpr FAR(word_t disp, word_t seg) noexcept : disp(disp), seg(seg) {}

    constexpr FAR& operator-= (sword_t rhs) noexcept {
        disp += rhs;
        return *this;
    }

    constexpr FAR& operator+= (sword_t rhs) noexcept {
        disp += rhs;
        return *this;
    }

    constexpr FAR operator-(sword_t rhs) const noexcept {
        return FAR { static_cast<word_t>(disp - rhs), seg };
    }

    constexpr FAR operator+(sword_t rhs) const noexcept {
        return FAR { static_cast<word_t>(disp + rhs), seg };
    }

    constexpr dword_t ea() const noexcept {
        return (disp + (seg << 4)) & 0xF'FF'FF;
    }
};
}

#endif // O126_COMMON_HPP

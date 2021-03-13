#pragma once
#include "../cpu.hpp"
#include <concepts>

struct o126::CPU::IMPL final {
    struct RM final {
        bool is_reg = {};
        REG reg = {};
        FAR ptr = {};
    };

    struct Opt_RM final {
        byte_t opt = {};
        RM rm = {};
    };

    struct REG_RM final {
        REG reg = {};
        RM rm = {};
    };

    struct SEG_RM final {
        SEG reg = {};
        RM rm = {};
    };

    template <typename type>
    struct ALU;
    struct BCD;

    struct CTX;
    template <typename type>
    struct MISC;
    struct Decode;
    struct EXE;

    /// Utility functions
    [[nodiscard]] static constexpr bool match8(char const(&data)[9], byte_t value) {
        std::uint8_t mask = 0x80;
        for (auto i = data; auto const c = *i; ++i) {
            if (c == '0') {
                if (value & mask) {
                    return false;
                }
            } else if (c == '1') {
                if (!(value & mask)) {
                    return false;
                }
            } else if (c < 'a' || c > 'z') {
                throw "Invalid mask character!";
            }
            mask >>= 1;
        }
        return true;
    }

    template <bool sign_extend = true, typename T>
    [[nodiscard]] static constexpr auto to_signed(T val) noexcept {
        if constexpr (sign_extend) {
            return static_cast<std::make_signed_t<T>>(val);
        } else {
            return static_cast<std::make_unsigned_t<T>>(val);
        }
    }
};

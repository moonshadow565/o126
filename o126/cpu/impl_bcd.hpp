#pragma once
#include "impl.hpp"
#include "impl_alu.hpp"

struct o126::CPU::IMPL::BCD final {
    using Result = typename ALU<byte_t>::Result;
    using Result2 = typename ALU<byte_t>::Result2;

    [[nodiscard]] static constexpr Result2 op_aaa(Flags flags, byte_t lo, byte_t hi) noexcept {
        if ((lo & 0xF) > 9 || flags.auxiliary) {
            lo += 6;
            hi += 1;
            flags.auxiliary = true;
            flags.carry = true;
        } else {
            flags.auxiliary = false;
            flags.carry = false;
        }
        lo &= 0xF;
        flags.parity = !(std::popcount(static_cast<byte_t>(lo)) & 1);
        flags.zero = static_cast<byte_t>(lo) == 0;
        flags.sign = false;
        return { flags, lo, hi };
    }

    [[nodiscard]] static constexpr Result2 op_aas(Flags flags, byte_t lo, byte_t hi) noexcept {
        if ((lo & 0xf) > 9 || flags.auxiliary) {
            lo -= 6;
            hi -= 1;
            flags.auxiliary = true;
            flags.carry = true;
        }  else {
            flags.auxiliary = false;
            flags.carry = false;
        }
        lo &= 0xF;
        flags.parity = !(std::popcount(static_cast<byte_t>(lo)) & 1);
        flags.zero = static_cast<byte_t>(lo) == 0;
        flags.sign = false;
        return { flags, lo, hi };
    }

    [[nodiscard]] static constexpr Result op_daa(Flags flags, byte_t lo) noexcept {
        if ((lo & 0xf) > 9 || flags.auxiliary) {
            if (lo > 0x99 || flags.carry) {
                lo += 0x60;
                flags.carry = true;
            } else {
                flags.carry = false;
            }
            lo += 0x06;
            flags.auxiliary = true;
        } else {
            if (lo > 0x99 || flags.carry) {
                lo += 0x60;
                flags.carry = true;
            } else {
                flags.carry = false;
            }
            flags.auxiliary = false;
        }
        return Result::with_parity_zero_sign(flags, lo);
    }

    [[nodiscard]] static constexpr Result op_das(Flags flags, byte_t lo) noexcept {
        if ((lo & 0xf) > 9 || flags.auxiliary) {
            if (lo > 0x99 || flags.carry) {
                lo -= 0x60;
                flags.carry = true;
            } else {
                flags.carry = lo <= 0x05;
            }
            lo -= 0x06;
            flags.auxiliary = true;
        } else {
            if (lo > 0x99 || flags.carry) {
                lo -= 0x60;
                flags.carry = true;
            } else {
                flags.carry = false;
            }
            flags.auxiliary = false;
        }
        return Result::with_parity_zero_sign(flags, lo);
    }

    [[nodiscard]] static constexpr Result2 op_aam(Flags flags, byte_t lhs_lo, byte_t rhs) noexcept {
        if (rhs == 0) {
            return {};
        }
        auto const lhs = static_cast<dword_t>(lhs_lo);
        auto const result = lhs / rhs;
        auto const remainder = lhs % rhs;

        flags.carry = false;
        flags.overflow = false;
        flags.auxiliary = false;
        flags.parity = !(std::popcount(static_cast<byte_t>(result)) & 1);
        flags.zero = static_cast<byte_t>(result) == 0;
        flags.sign = (result >> 7) & 1;

        return { flags, static_cast<byte_t>(remainder), static_cast<byte_t>(result) };
    }

    [[nodiscard]] static constexpr Result2 op_aad(Flags flags, byte_t lhs_lo, byte_t lhs_hi, byte_t rhs) noexcept {
        auto const result = lhs_lo + (lhs_hi * rhs);

        flags.carry = false;
        flags.overflow = false;
        flags.auxiliary = false;
        flags.parity = !(std::popcount(static_cast<byte_t>(result)) & 1);
        flags.zero = static_cast<byte_t>(result) == 0;
        flags.sign = (result >> 7) & 1;

        return { flags, static_cast<byte_t>(result), 0 };
    }
};

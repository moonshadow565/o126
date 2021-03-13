#pragma once
#include "impl.hpp"
#include <algorithm>
#include <bit>
#include <limits>

template<typename type>
struct o126::CPU::IMPL::ALU final {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
    static constexpr byte_t BIT_COUNT = std::numeric_limits<type>::digits;
    static constexpr byte_t BIT_NEXT = BIT_COUNT + 1;
    static constexpr byte_t BIT_LAST = BIT_COUNT - 1;
    using stype = std::make_signed_t<type>;
    static constexpr stype SIGNED_MIN = std::numeric_limits<stype>::min();
    static constexpr stype SIGNED_MAX = std::numeric_limits<stype>::max();

    struct Result {
        Flags flags = {};
        type value = {};

        [[nodiscard]] static constexpr Result with_parity_zero_sign(Flags flags, type value) noexcept {
            flags.parity = !(std::popcount(static_cast<byte_t>(value)) & 1);
            flags.zero = static_cast<type>(value) == 0;
            flags.sign = (value >> BIT_LAST) & 1;
            return { flags, value };
        }
    };

    struct Result2 {
        bool error = {};
        Flags flags = {};
        type value = {};
        type value2 = {};

        constexpr Result2() noexcept : error(true) {}
        constexpr Result2(Flags flags, type value, type value2) noexcept
            : error(false), flags(flags), value(value), value2(value2) {}
    };

    [[nodiscard]] static constexpr Result op_add(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs + rhs;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) + (rhs & ~(1 << BIT_LAST));
        auto const result_aux = (lhs & 0xF) + (rhs & 0xF);

        flags.carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ flags.carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_or(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs | rhs;

        flags.carry = false;
        flags.overflow = false;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_adc(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs + rhs + flags.carry;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) + (rhs & ~(1 << BIT_LAST)) + flags.carry;
        auto const result_aux = (lhs & 0xF) + (rhs & 0xF) + flags.carry;

        flags.carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ flags.carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_sbb(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs - rhs - flags.carry;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) - (rhs & ~(1 << BIT_LAST)) - flags.carry;
        auto const result_aux = (lhs & 0xF) - (rhs & 0xF) - flags.carry;

        flags.carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ flags.carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_and(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs & rhs;

        flags.carry = false;
        flags.overflow = false;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_sub(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs - rhs;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) - (rhs & ~(1 << BIT_LAST));
        auto const result_aux = (lhs & 0xF) - (rhs & 0xF);

        flags.carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ flags.carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_xor(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs ^ rhs;

        flags.carry = false;
        flags.overflow = false;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_cmp(Flags flags, type lhs, type rhs) noexcept {
        return op_sub(flags, lhs, rhs);
    }

    [[nodiscard]] static constexpr Result op_test(Flags flags, type lhs, type rhs) noexcept {
        return op_and(flags, lhs, rhs);
    }

    static constexpr Result (* const table_alu[8])(Flags flags, type lhs, type rhs) noexcept = {
        &op_add, &op_or, &op_adc, &op_sbb, &op_and, &op_sub, &op_xor, &op_cmp,
    };

    [[nodiscard]] static constexpr Result op_rol(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs %= BIT_COUNT;
            auto const result = std::rotl(lhs, rhs % BIT_COUNT);

            flags.carry = result & 1;
            flags.overflow = (flags.carry ^ (result >> BIT_LAST)) & 1;

            return { flags, static_cast<type>(result) };
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_ror(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs %= BIT_COUNT;
            auto const result = std::rotr(lhs, rhs % BIT_COUNT);

            flags.carry = (result >> BIT_LAST) & 1;
            flags.overflow = (((result << 1) ^ result) >> BIT_LAST) & 1;

            return { flags, static_cast<type>(result) };
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_rcl(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs %= BIT_NEXT;
            auto const lhs_c = (flags.carry << BIT_COUNT) | lhs;
            auto const result = (lhs_c << rhs) | (lhs_c >> (BIT_NEXT - rhs));

            flags.carry = (result >> BIT_COUNT) & 1;
            flags.overflow = (flags.carry ^ (result >> BIT_LAST)) & 1;

            return { flags, static_cast<type>(result) };
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_rcr(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs %= BIT_NEXT;
            auto const lhs_c = (flags.carry << BIT_COUNT) | lhs;
            auto const result = (lhs_c >> rhs) | (lhs_c << (BIT_NEXT - rhs));

            flags.carry = (result >> BIT_COUNT) & 1;
            flags.overflow = (((result << 1) ^ result) >> BIT_LAST) & 1;

            return { flags, static_cast<type>(result) };
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_shl(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs = std::min(rhs, BIT_NEXT);
            auto const result = lhs << rhs;

            flags.carry = (result >> BIT_COUNT) & 1;
            flags.overflow = ((result >> BIT_LAST) & 1) ^ flags.carry;
            flags.auxiliary = false;

            return Result::with_parity_zero_sign(flags, static_cast<type>(result));
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_shr(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs = std::min(rhs, BIT_NEXT);
            auto const result = lhs >> rhs;

            flags.carry = (lhs >> (rhs - 1)) & 1;
            flags.overflow = (lhs >> BIT_LAST) & 1;
            flags.auxiliary = false;
            return Result::with_parity_zero_sign(flags, static_cast<type>(result));
        }
        return { flags, lhs };
    }

    [[nodiscard]] static constexpr Result op_sal(Flags flags, type lhs, byte_t rhs) noexcept {
        return op_shl(flags, lhs, rhs);
    }

    [[nodiscard]] static constexpr Result op_sar(Flags flags, type lhs, byte_t rhs) noexcept {
        if (rhs %= 32) {
            rhs = std::min(rhs, BIT_NEXT);
            auto const result = static_cast<stype>(lhs) >> rhs;

            flags.carry = (static_cast<stype>(lhs) >> (rhs - 1)) & 1;
            flags.overflow = false;
            flags.auxiliary = false;

            return Result::with_parity_zero_sign(flags, static_cast<type>(result));
        }
        return { flags, lhs };
    }

    static constexpr Result (* const table_rot[8])(Flags flags, type lhs, byte_t rhs) noexcept = {
        &op_rol, &op_ror, &op_rcl, &op_rcr, &op_shl, &op_shr, &op_sal, &op_sar,
    };

    [[nodiscard]] static constexpr Result op_neg(Flags flags, type rhs) noexcept {
        constexpr auto const lhs = static_cast<type>(0);
        auto const result = lhs - rhs;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) - (rhs & ~(1 << BIT_LAST));
        auto const result_aux = (lhs & 0xF) - (rhs & 0xF);

        flags.carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ flags.carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_inc(Flags flags, type lhs) noexcept {
        constexpr auto const rhs = static_cast<type>(1);
        auto const result = lhs + rhs;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) + (rhs & ~(1 << BIT_LAST));
        auto const result_aux = (lhs & 0xF) + (rhs & 0xF);

        auto const carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result op_dec(Flags flags, type lhs) noexcept {
        constexpr auto const rhs = static_cast<type>(1);
        auto const result = lhs - rhs;
        auto const result_overflow = (lhs & ~(1 << BIT_LAST)) - (rhs & ~(1 << BIT_LAST));
        auto const result_aux = (lhs & 0xF) - (rhs & 0xF);

        auto const carry = (result >> BIT_COUNT) & 1;
        flags.overflow = ((result_overflow >> BIT_LAST) ^ carry) & 1;
        flags.auxiliary = (result_aux >> 4) & 1;

        return Result::with_parity_zero_sign(flags, static_cast<type>(result));
    }

    [[nodiscard]] static constexpr Result2 op_mul(Flags flags, type lhs, type rhs) noexcept {
        auto const result = lhs * rhs;

        flags.carry = static_cast<type>(result >> BIT_COUNT);
        flags.overflow = flags.carry;

        flags.parity = !(std::popcount(static_cast<type>(result)) & 1);
        flags.zero = static_cast<type>(result) == 0;
        flags.sign = (result >> BIT_LAST) & 1;

        return { flags, static_cast<type>(result), static_cast<type>(result >> BIT_COUNT) };
    }

    [[nodiscard]] static constexpr Result2 op_imul(Flags flags, stype lhs, stype rhs) noexcept {
        auto const result = lhs * rhs;

        flags.carry = (result < SIGNED_MIN) || (result > SIGNED_MAX);
        flags.overflow = flags.carry;

        flags.parity = !(std::popcount(static_cast<type>(result)) & 1);
        flags.zero = static_cast<type>(result) == 0;
        flags.sign = (result >> BIT_LAST) & 1;

        return { flags, static_cast<type>(result), static_cast<type>(result >> BIT_COUNT) };
    }

    [[nodiscard]] static constexpr Result2 op_div(Flags flags, type lhs_lo, type lhs_hi, type rhs) noexcept {
        if (rhs == 0) {
            return {};
        }
        auto const lhs = static_cast<dword_t>(lhs_lo | (lhs_hi << BIT_COUNT));
        auto const result = lhs / rhs;
        auto const remainder = lhs % rhs;
        if (static_cast<type>(result >> BIT_COUNT)) {
            return {};
        }

        return { flags, static_cast<type>(result), static_cast<type>(remainder) };
    }

    [[nodiscard]] static constexpr Result2 op_idiv(Flags flags, type lhs_lo, stype lhs_hi, stype rhs) noexcept {
        if (rhs == 0) {
            return {};
        }
        auto const lhs = static_cast<sdword_t>(lhs_lo | (lhs_hi << BIT_COUNT));
        auto const result = lhs / to_signed(rhs);
        auto const remainder = lhs % to_signed(rhs);
        if ((result < SIGNED_MIN) || (result > SIGNED_MAX)) {
            return {};
        }

        return { flags, static_cast<type>(result), static_cast<type>(remainder) };
    }
#pragma clang diagnostic pop
};

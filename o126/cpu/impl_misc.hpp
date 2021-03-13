#pragma once
#include "impl.hpp"
#include "impl_ctx.hpp"
#include <bit>
#include <limits>

template<typename type>
struct o126::CPU::IMPL::MISC final {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
    [[nodiscard]] static constexpr Result op1_test(CTX ctx, RM rm) noexcept {
        auto const rhs = ctx.fetch<type>();
        auto const lhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_test(flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_reserved(CTX ctx, RM rm) noexcept {
        return op1_test(ctx, rm);
    }

    [[nodiscard]] static constexpr Result op1_not(CTX ctx, RM rm) noexcept {
        auto const lhs = ctx.rm_get<type>(rm);
        auto const result = static_cast<type>(~lhs);
        ctx.rm_set<type>(rm, result);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_neg(CTX ctx, RM rm) noexcept {
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_neg(flags, rhs);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<type>(rm, result.value);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_mul(CTX ctx, RM rm) noexcept {
        auto const lhs = ctx.reg_get<type>(REG::AX);
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_mul(flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<type>(result.value, result.value2);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_imul(CTX ctx, RM rm) noexcept {
        auto const lhs = ctx.reg_get<type>(REG::AX);
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_imul(flags, to_signed(lhs), to_signed(rhs));
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<type>(result.value, result.value2);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_div(CTX ctx, RM rm) noexcept {
        auto const [lhs_lo, lhs_hi] = ctx.pair_get<type>();
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_div(flags, lhs_lo, lhs_hi, rhs);
        if (result.error) {
            ctx.inst_repeat();
            ctx.push_frame_interupt();
            return ctx.end_interupt(0);
        }
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<type>(result.value, result.value2);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op1_idiv(CTX ctx, RM rm) noexcept {
        auto const [lhs_lo, lhs_hi] = ctx.pair_get<type>();
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_idiv(flags, lhs_lo, to_signed(lhs_hi), to_signed(rhs));
        if (result.error) {
            ctx.inst_repeat();
            ctx.push_frame_interupt();
            return ctx.end_interupt(0);
        }
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<type>(result.value, result.value2);
        return ctx.end_next();
    }

    static constexpr Result (* const table_misc1[8])(CTX ctx, RM rm) noexcept = {
        &op1_test, &op1_reserved, &op1_not, &op1_neg, &op1_mul, &op1_imul, &op1_div, &op1_idiv,
    };

    [[nodiscard]] static constexpr Result op2_inc(CTX ctx, RM rm) noexcept {
        auto const value = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_inc(flags, value);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<word_t>(rm, result.value);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op2_dec(CTX ctx, RM rm) noexcept {
        auto const value = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::op_dec(flags, value);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<word_t>(rm, result.value);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op2_call_near(CTX ctx, RM rm) noexcept {
        if constexpr (!std::is_same_v<type, word_t>) {
            return ctx.end_bad();
        }
        auto const addr_next = ctx.rm_get<word_t>(rm);
        ctx.push_frame_near();
        return ctx.end_jmp_near(addr_next);
    }

    [[nodiscard]] static constexpr Result op2_call_far(CTX ctx, RM rm) noexcept {
        if constexpr (!std::is_same_v<type, word_t>) {
            return ctx.end_bad();
        }
        auto const addr_next = ctx.rm_get<FAR>(rm);
        ctx.push_frame_far();
        return ctx.end_jmp_far(addr_next);
    }

    [[nodiscard]] static constexpr Result op2_jmp_near(CTX ctx, RM rm) noexcept {
        if constexpr (!std::is_same_v<type, word_t>) {
            return ctx.end_bad();
        }
        auto const addr_next = ctx.rm_get<word_t>(rm);
        return ctx.end_jmp_near(addr_next);
    }

    [[nodiscard]] static constexpr Result op2_jmp_far(CTX ctx, RM rm) noexcept {
        if constexpr (!std::is_same_v<type, word_t>) {
            return ctx.end_bad();
        }
        auto const addr_next = ctx.rm_get<FAR>(rm);
        return ctx.end_jmp_far(addr_next);
    }

    [[nodiscard]] static constexpr Result op2_push(CTX ctx, RM rm) noexcept {
        if constexpr (!std::is_same_v<type, word_t>) {
            return ctx.end_bad();
        }
        // NOTE: push SP decrements SP before pushing
        auto const addr = ctx.ptr_inc_pre(REG::SP, SEG::SS, -2);
        auto const value = ctx.rm_get<word_t>(rm);
        ctx.mem_set<word_t>(addr, value);
        return ctx.end_next();
    }

    [[nodiscard]] static constexpr Result op2_reserved(CTX ctx, RM rm) noexcept {
        (void)rm;
        return ctx.end_bad();
    }

    static constexpr Result (* const table_misc2[8])(CTX ctx, RM rm) noexcept = {
        &op2_inc, &op2_dec, &op2_call_near, &op2_call_far, &op2_jmp_near, &op2_jmp_far, &op2_push, &op2_reserved,
    };
#pragma clang diagnostic pop
};

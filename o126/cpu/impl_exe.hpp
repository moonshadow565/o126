#pragma once
#include "impl.hpp"
#include "impl_alu.hpp"
#include "impl_bcd.hpp"
#include "impl_ctx.hpp"
#include "impl_decode.hpp"
#include "impl_misc.hpp"

struct o126::CPU::IMPL::EXE final {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
    // MOV rmW, rW
    template <byte_t OP> requires(match8("1000100w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const value = ctx.reg_get<type>(reg);
        ctx.rm_set<type>(rm, value);
        return ctx.end_next();
    }

    // MOV rW, rmW
    template <byte_t OP> requires(match8("1000101w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const value = ctx.rm_get<type>(rm);
        ctx.reg_set<type>(reg, value);
        return ctx.end_next();
    }

    // MOV aW, memW
    template <byte_t OP> requires(match8("1010000w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const disp = Decode::imm<word_t>(ctx);
        auto const seg = ctx.seg_get(SEG::DS_OR_PREFIX);
        auto const addr = FAR { disp, seg };
        auto const value = ctx.mem_get<type>(addr);
        ctx.reg_set<type>(REG::AX, value);
        return ctx.end_next();
    }

    // MOV memW, aW
    template <byte_t OP> requires(match8("1010001w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const disp = Decode::imm<word_t>(ctx);
        auto const seg = ctx.seg_get(SEG::DS_OR_PREFIX);
        auto const addr = FAR { disp, seg };
        auto const value = ctx.reg_get<type>(REG::AX);
        ctx.mem_set<type>(addr, value);
        return ctx.end_next();
    }

    // MOV sr, rm16
    template <byte_t OP> requires(match8("10001110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [seg, rm] = Decode::seg_rm(ctx);
        auto const value = ctx.rm_get<word_t>(rm);
        ctx.seg_set(seg, value);
        return ctx.end_next();
    }

    // MOV rm16, sr16
    template <byte_t OP> requires(match8("10001100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [seg, rm] = Decode::seg_rm(ctx);
        auto const value = ctx.seg_get(seg);
        ctx.rm_set<word_t>(rm, value);
        return ctx.end_next();
    }

    // MOV rmW, immW
    template <byte_t OP> requires(match8("1100011w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [opt, rm] = Decode::opt_rm(ctx);
        auto const imm = Decode::imm<type>(ctx);
        ctx.rm_set<type>(rm, imm);
        return ctx.end_next();
    }

    // MOV rW, immW
    template <byte_t OP> requires(match8("1011wreg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1000>;
        constexpr auto const reg = static_cast<REG>(OP & 7);
        auto const imm = Decode::imm<type>(ctx);
        ctx.reg_set<type>(reg, imm);
        return ctx.end_next();
    }

    // PUSH r16
    template <byte_t OP> requires(match8("01010reg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const reg = static_cast<REG>(OP & 7);
        // NOTE: push SP decrements SP before pushing
        auto const addr = ctx.ptr_inc_pre(REG::SP, SEG::SS, -2);
        auto const value = ctx.reg_get<word_t>(reg);
        ctx.mem_set<word_t>(addr, value);
        return ctx.end_next();
    }

    // PUSH sr
    template <byte_t OP> requires(match8("000sr110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const seg = static_cast<SEG>((OP >> 3) & 3);
        auto const value = ctx.seg_get(seg);
        ctx.push<word_t>(value);
        return ctx.end_next();
    }

    // POP rm16
    template <byte_t OP> requires(match8("10001111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [opt, rm] = Decode::opt_rm(ctx);
        auto const value = ctx.pop<word_t>();
        ctx.rm_set<word_t>(rm, value);
        return ctx.end_next();
    }

    // POP r16
    template <byte_t OP> requires(match8("01011reg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const reg = static_cast<REG>(OP & 7);
        // NOTE: pop SP sets the value after incrementing SP
        auto const value = ctx.pop<word_t>();
        ctx.reg_set<word_t>(reg, value);
        return ctx.end_next();
    }

    // POP sr
    template <byte_t OP> requires(match8("000sr111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const seg = static_cast<SEG>((OP >> 3) & 3);
        auto const value = ctx.pop<word_t>();
        ctx.seg_set(seg, value);
        return ctx.end_next();
    }

    // XCHG rW, rmW
    template <byte_t OP> requires(match8("1000011w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const value1 = ctx.reg_get<type>(reg);
        auto const value2 = ctx.rm_get<type>(rm);
        ctx.rm_set<type>(rm, value2);
        ctx.reg_set<type>(reg, value1);
        return ctx.end_next();
    }

    // XCHG r16, a16
    template <byte_t OP> requires(match8("10010reg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const reg = static_cast<REG>(OP & 7);
        auto const value1 = ctx.reg_get<word_t>(REG::AX);
        auto const value2 = ctx.reg_get<word_t>(reg);
        ctx.reg_set<word_t>(REG::AX, value2);
        ctx.reg_set<word_t>(reg, value1);
        return ctx.end_next();
    }

    // IN aW, im8
    template <byte_t OP> requires(match8("1110010w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const port = Decode::imm<byte_t>(ctx);
        auto const value = ctx.port_get<type>(port);
        ctx.reg_set<type>(REG::AX, value);
        return ctx.end_next();
    }

    // IN aW, d16
    template <byte_t OP> requires(match8("1110110w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const port = ctx.reg_get<word_t>(REG::DX);
        auto const value = ctx.port_get<type>(port);
        ctx.reg_set<type>(REG::AX, value);
        return ctx.end_next();
    }

    // OUT im8, aW
    template <byte_t OP> requires(match8("1110011w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const port = Decode::imm<byte_t>(ctx);
        auto const value = ctx.reg_get<type>(REG::AX);
        ctx.port_set<type>(port, value);
        return ctx.end_next();
    }

    // OUT aW, d16
    template <byte_t OP> requires(match8("1110111w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const port = ctx.reg_get<word_t>(REG::DX);
        auto const value = ctx.reg_get<type>(REG::AX);
        ctx.port_set<type>(port, value);
        return ctx.end_next();
    }

    // XLAT a16, [b16 + a8]
    template <byte_t OP> requires(match8("11010111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto disp = ctx.reg_get<word_t>(REG::BX);
        disp += ctx.reg_get<byte_t>(REG::AL);
        auto const seg = ctx.seg_get(SEG::DS_OR_PREFIX);
        auto const addr = FAR { disp, seg };
        auto const value = ctx.mem_get<byte_t>(addr);
        ctx.reg_set<byte_t>(REG::AL, value);
        return ctx.end_next();
    }

    // LEA r16, rm16
    template <byte_t OP> requires(match8("10001101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [reg, rm] = Decode::reg_rm(ctx);
        if (rm.is_reg) {
            return ctx.end_bad();
        }
        ctx.reg_set<word_t>(reg, rm.ptr.disp);
        return ctx.end_next();
    }

    // LDS ds:r16, mem32
    template <byte_t OP> requires(match8("11000101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [reg, rm] = Decode::reg_rm(ctx);
        if (rm.is_reg) {
            return ctx.end_bad();
        }
        auto const value = ctx.rm_get<FAR>(rm);
        ctx.reg_set<word_t>(reg, value.disp);
        ctx.seg_set(SEG::DS, value.seg);
        return ctx.end_next();
    }

    // LES es:r16, mem32
    template <byte_t OP> requires(match8("11000100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [reg, rm] = Decode::reg_rm(ctx);
        if (rm.is_reg) {
            return ctx.end_bad();
        }
        auto const value = ctx.rm_get<FAR>(rm);
        ctx.reg_set<word_t>(reg, value.disp);
        ctx.seg_set(SEG::ES, value.seg);
        return ctx.end_next();
    }

    // LAHF a8h
    template <byte_t OP> requires(match8("10011111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.flags_get<byte_t>();
        ctx.reg_set<byte_t>(REG::AH, value);
        return ctx.end_next();
    }

    // SAHF
    template <byte_t OP> requires(match8("10011110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.reg_get<byte_t>(REG::AH);
        ctx.flags_set<byte_t>(value);
        return ctx.end_next();
    }

    // PUSHF
    template <byte_t OP> requires(match8("10011100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.flags_get<word_t>();
        ctx.push<word_t>(value);
        return ctx.end_next();
    }

    // POPF
    template <byte_t OP> requires(match8("10011101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.pop<word_t>();
        ctx.flags_set<word_t>(value);
        return ctx.end_next();
    }

    // INC r
    template <byte_t OP> requires(match8("01000reg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const reg = static_cast<REG>(OP & 7);
        auto const flags = ctx.flags_get<Flags>();
        auto const value = ctx.reg_get<word_t>(reg);
        auto const result = ALU<word_t>::op_inc(flags, value);
        ctx.flags_set<Flags>(result.flags);
        ctx.reg_set<word_t>(reg, result.value);
        return ctx.end_next();
    }

    // DEC r
    template <byte_t OP> requires(match8("01001reg", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const reg = static_cast<REG>(OP & 7);
        auto const flags = ctx.flags_get<Flags>();
        auto const value = ctx.reg_get<word_t>(reg);
        auto const result = ALU<word_t>::op_dec(flags, value);
        ctx.flags_set<Flags>(result.flags);
        ctx.reg_set<word_t>(reg, result.value);
        return ctx.end_next();
    }

    // AAA
    template <byte_t OP> requires(match8("00110111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [lo, hi] = ctx.pair_get<byte_t>();
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_aaa(flags, lo, hi);
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<byte_t>(result.value, result.value2);
        return ctx.end_next();
    }

    // AAS
    template <byte_t OP> requires(match8("00111111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [lo, hi] = ctx.pair_get<byte_t>();
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_aas(flags, lo, hi);
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<byte_t>(result.value, result.value2);
        return ctx.end_next();
    }

    // DAA
    template <byte_t OP> requires(match8("00100111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const lo = ctx.reg_get<byte_t>(REG::AL);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_daa(flags, lo);
        ctx.flags_set<Flags>(result.flags);
        ctx.reg_set<byte_t>(REG::AL, result.value);
        return ctx.end_next();
    }

    // DAS
    template <byte_t OP> requires(match8("00101111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const lo = ctx.reg_get<byte_t>(REG::AL);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_das(flags, lo);
        ctx.flags_set<Flags>(result.flags);
        ctx.reg_set<byte_t>(REG::AL, result.value);
        return ctx.end_next();
    }

    // AAM im
    template <byte_t OP> requires(match8("11010100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const imm = Decode::imm<byte_t>(ctx);
        auto const [lo, hi] = ctx.pair_get<byte_t>();
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_aam(flags, lo, imm);
        if (result.error) {
            ctx.inst_repeat();
            ctx.push_frame_interupt();
            return ctx.end_interupt(0);
        }
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<byte_t>(result.value, result.value2);
        return ctx.end_next();
    }

    // AAD im
    template <byte_t OP> requires(match8("11010101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const imm = Decode::imm<byte_t>(ctx);
        auto const [lo, hi] = ctx.pair_get<byte_t>();
        auto const flags = ctx.flags_get<Flags>();
        auto const result = BCD::op_aad(flags, lo, hi, imm);
        ctx.flags_set<Flags>(result.flags);
        ctx.pair_set<byte_t>(result.value, result.value2);
        return ctx.end_next();
    }

    // SALC
    template <byte_t OP> requires(match8("11010110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const flags = ctx.flags_get<Flags>();
        if (flags.carry) {
            ctx.reg_set<byte_t>(REG::AL, static_cast<byte_t>(-1));
        } else {
            ctx.reg_set<byte_t>(REG::AL, static_cast<byte_t>(0));
        }
        return ctx.end_next();
    }

    // CBW a, a
    template <byte_t OP> requires(match8("10011000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.reg_get<byte_t>(REG::AL);
        auto const result = to_signed(value);
        if (result < 0) {
            ctx.reg_set<byte_t>(REG::AH, static_cast<byte_t>(-1));
        } else {
            ctx.reg_set<byte_t>(REG::AH, 0);
        }
        return ctx.end_next();
    }

    // CWD a:d, a
    template <byte_t OP> requires(match8("10011001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const value = ctx.reg_get<word_t>(REG::AX);
        auto const result = to_signed(value);
        if (result < 0) {
            ctx.reg_set<word_t>(REG::DX, static_cast<word_t>(-1));
        } else {
            ctx.reg_set<word_t>(REG::DX, static_cast<word_t>(0));
        }
        return ctx.end_next();
    }

    // TEST r, rm
    template <byte_t OP> requires(match8("1000010w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const lhs = ctx.reg_get<type>(reg);
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<word_t>::op_test(flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        return ctx.end_next();
    }

    // TEST a, imm
    template <byte_t OP> requires(match8("1010100w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const lhs = Decode::imm<type>(ctx);
        auto const rhs = ctx.reg_get<type>(REG::AX);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<word_t>::op_test(flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        return ctx.end_next();
    }

    // REP
    template <byte_t OP> requires(match8("1111001z", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const is_zero = static_cast<bool>(OP & 0b01);
        if constexpr (is_zero) {
            return ctx.end_prefix_rep(REP::ZERO);
        } else {
            return ctx.end_prefix_rep(REP::NOT_ZERO);
        }
    }

    // MOVS
    template <byte_t OP> requires(match8("1010010w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const addr_src = ctx.str_src<type>();
            auto const addr_dst = ctx.str_dst<type>();
            auto const value = ctx.mem_get<type>(addr_src);
            ctx.mem_set<type>(addr_dst, value);
            return true;
        });
    }

    // CMPS
    template <byte_t OP> requires(match8("1010011w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const addr_src = ctx.str_src<type>();
            auto const addr_dst = ctx.str_dst<type>();
            auto const lhs = ctx.mem_get<type>(addr_src);
            auto const rhs = ctx.mem_get<type>(addr_dst);
            auto const flags = ctx.flags_get<Flags>();
            auto const result = ALU<type>::op_cmp(flags, lhs, rhs);
            ctx.flags_set<Flags>(result.flags);
            return ctx.str_rep();
        });
    }

    // SCAS
    template <byte_t OP> requires(match8("1010111w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const addr_dst = ctx.str_dst<type>();
            auto const lhs = ctx.reg_get<type>(REG::AX);
            auto const rhs = ctx.mem_get<type>(addr_dst);
            auto const flags = ctx.flags_get<Flags>();
            auto const result = ALU<type>::op_cmp(flags, lhs, rhs);
            ctx.flags_set<Flags>(result.flags);
            return ctx.str_rep();
        });
    }

    // LODS
    template <byte_t OP> requires(match8("1010110w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const addr_src = ctx.str_src<type>();
            auto const value = ctx.mem_get<type>(addr_src);
            ctx.reg_set<type>(REG::AX, value);
            return true;
        });
    }

    // STOS
    template <byte_t OP> requires(match8("1010101w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const addr_dst = ctx.str_dst<type>();
            auto const value = ctx.reg_get<type>(REG::AX);
            ctx.mem_set<type>(addr_dst, value);
            return true;
        });
    }

    // CALL im
    template <byte_t OP> requires(match8("11101000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const disp = Decode::rel<word_t>(ctx);
        ctx.push_frame_near();
        return ctx.end_jmp_rel(disp);
    }

    // CALLI im
    template <byte_t OP> requires(match8("10011010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const addr_next = Decode::addr<FAR>(ctx);
        ctx.push_frame_far();
        return ctx.end_jmp_far(addr_next);
    }

    // JMP rel16
    template <byte_t OP> requires(match8("11101001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const disp = Decode::rel<word_t>(ctx);
        return ctx.end_jmp_rel(disp);
    }

    // JMP rel8
    template <byte_t OP> requires(match8("11101011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const disp = Decode::rel<byte_t>(ctx);
        return ctx.end_jmp_rel(disp);
    }

    // JMPI im
    template <byte_t OP> requires(match8("11101010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const addr_next = Decode::addr<FAR>(ctx);
        return ctx.end_jmp_far(addr_next);
    }

    // RET
    template <byte_t OP> requires(match8("11000011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const addr_next = ctx.pop_frame_near();
        return ctx.end_jmp_near(addr_next);
    }

    // RET im
    template <byte_t OP> requires(match8("11000010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const imm = Decode::rel<word_t>(ctx);
        auto const addr_next = ctx.pop_frame_near();
        ctx.reg_add(REG::SP, imm);
        return ctx.end_jmp_near(addr_next);
    }

    // RETI
    template <byte_t OP> requires(match8("11001011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const addr_next = ctx.pop_frame_far();
        return ctx.end_jmp_far(addr_next);
    }

    // RETI im
    template <byte_t OP> requires(match8("11001010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const imm = Decode::rel<word_t>(ctx);
        auto const addr_next = ctx.pop_frame_far();
        ctx.reg_add(REG::SP, imm);
        return ctx.end_jmp_far(addr_next);
    }

    // JNE/JNZ          ZF=0
    template <byte_t OP> requires(match8("0111010f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.zero == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNL/JGE          SF=OF
    template <byte_t OP> requires(match8("0111110f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.sign == flags.overflow) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNLE/JG          ZF=0 && SF=OF
    template <byte_t OP> requires(match8("0111111f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.zero == false && flags.sign == flags.overflow) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNB/JNC/JAE      CF=0
    template <byte_t OP> requires(match8("0111001f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.carry == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNBE/JA          CF=0 && ZF=0
    template <byte_t OP> requires(match8("0111011f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.carry == false && flags.zero == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNP              PF=0
    template <byte_t OP> requires(match8("0111101f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.parity == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNO              OF=0
    template <byte_t OP> requires(match8("0111000f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.overflow == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JNS              SF=0
    template <byte_t OP> requires(match8("0111100f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        if ((flags.sign == false) == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // LOOP
    template <byte_t OP> requires(match8("11100010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const disp = Decode::rel<byte_t>(ctx);
        auto count = ctx.reg_get<word_t>(REG::CX);
        --count;
        ctx.reg_set<word_t>(REG::CX, count);
        if (count != 0) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // LOOPZ
    template <byte_t OP> requires(match8("1110000f", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const condition = static_cast<bool>(OP & 1);
        auto const disp = Decode::rel<byte_t>(ctx);
        auto const flags = ctx.flags_get<Flags>();
        auto count = ctx.reg_get<word_t>(REG::CX);
        --count;
        ctx.reg_set<word_t>(REG::CX, count);
        if (count != 0 && flags.zero == condition) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // JCXZ
    template <byte_t OP> requires(match8("11100011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const disp = Decode::rel<byte_t>(ctx);
        auto count = ctx.reg_get<word_t>(REG::CX);
        if (count == 0) {
            return ctx.end_jmp_rel(disp);
        }
        return ctx.end_next();
    }

    // INT t
    template <byte_t OP> requires(match8("11001101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const index = Decode::imm<byte_t>(ctx);
        ctx.push_frame_interupt();
        return ctx.end_interupt(index);
    }

    // INT 3
    template <byte_t OP> requires(match8("11001100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        ctx.push_frame_interupt();
        return ctx.end_interupt(3);
    }

    // INTO
    template <byte_t OP> requires(match8("11001110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const flags = ctx.flags_get<Flags>();
        if (flags.overflow) {
            ctx.push_frame_interupt();
            return ctx.end_interupt(4);
        }
        return ctx.end_next();
    }

    // IRET
    template <byte_t OP> requires(match8("11001111", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const addr_next = ctx.pop_frame_interupt();
        return ctx.end_jmp_far(addr_next);
    }

    // CLC
    template <byte_t OP> requires(match8("11111000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.carry = false;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // CMC
    template <byte_t OP> requires(match8("11110101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.carry = !flags.carry;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // STC
    template <byte_t OP> requires(match8("11111001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.carry = true;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // CLD
    template <byte_t OP> requires(match8("11111100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.direction = false;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // STD
    template <byte_t OP> requires(match8("11111101", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.direction = true;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // CLI
    template <byte_t OP> requires(match8("11111010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.interupt = false;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // STI
    template <byte_t OP> requires(match8("11111011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto flags = ctx.flags_get<Flags>();
        flags.interupt = true;
        ctx.flags_set<Flags>(flags);
        return ctx.end_next();
    }

    // HLT
    template <byte_t OP> requires(match8("11110100", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_halt();
    }

    // WAIT
    template <byte_t OP> requires(match8("10011011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_wait();
    }

    // ESC rm
    template <byte_t OP> requires(match8("11011xxx", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [opt, rm] = Decode::opt_rm(ctx);
        (void)opt;
        (void)rm;
        // TODO: do what here?
        return ctx.end_next();
    }

    // LOCK
    template <byte_t OP> requires(match8("11110000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_prefix_lock();
    }

    // SEG:
    template <byte_t OP> requires(match8("001sr110", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const seg = static_cast<SEG>((OP >> 3) & 0b11);
        return ctx.end_prefix_seg(seg);
    }

    // ALU_OP rm, r
    template <byte_t OP> requires(match8("00alu00w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const alu = (OP >> 3) & 7;
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const lhs = ctx.rm_get<type>(rm);
        auto const rhs = ctx.reg_get<type>(reg);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_alu[alu](flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        if constexpr (alu != 7) {
            ctx.rm_set<type>(rm, result.value);
        }
        return ctx.end_next();
    }

    // ALU_OP r, rm
    template <byte_t OP> requires(match8("00alu01w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const alu = (OP >> 3) & 7;
        using type = Decode::type<OP & 0b1>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const lhs = ctx.reg_get<type>(reg);
        auto const rhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_alu[alu](flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        if constexpr (alu != 7) {
            ctx.reg_set<type>(reg, result.value);
        }
        return ctx.end_next();
    }

    // ALU_OP a, imm
    template <byte_t OP> requires(match8("00alu10w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        constexpr auto const alu = (OP >> 3) & 7;
        using type = Decode::type<OP & 0b1>;
        auto const imm = Decode::imm<type>(ctx);
        auto const lhs = ctx.reg_get<type>(REG::AX);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_alu[alu](flags, lhs, imm);
        ctx.flags_set<Flags>(result.flags);
        if constexpr (alu != 7) {
            ctx.reg_set<type>(REG::AX, result.value);
        }
        return ctx.end_next();
    }

    // ALU_OP rm8, imm8
    template <byte_t OP> requires(match8("100000s0", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [alu, rm] = Decode::opt_rm(ctx);
        auto const imm = Decode::imm<byte_t>(ctx);
        auto const lhs = ctx.rm_get<byte_t>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<byte_t>::table_alu[alu](flags, lhs, imm);
        ctx.flags_set<Flags>(result.flags);
        if (alu != 7) {
            ctx.rm_set<byte_t>(rm, result.value);
        }
        return ctx.end_next();
    }

    // ALU_OP rm16, immS
    template <byte_t OP> requires(match8("100000s1", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<!(OP & 0b10)>;
        auto const [alu, rm] = Decode::opt_rm(ctx);
        auto const imm = Decode::imm<type>(ctx);
        auto const lhs = ctx.rm_get<word_t>(rm);
        auto const rhs = static_cast<word_t>(to_signed(imm));
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<word_t>::table_alu[alu](flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        if (alu != 7) {
            ctx.rm_set<word_t>(rm, result.value);
        }
        return ctx.end_next();
    }

    // ROT_OP rm, 1
    template <byte_t OP> requires(match8("1101000w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [rot, rm] = Decode::opt_rm(ctx);
        auto const lhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_rot[rot](flags, lhs, 1);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<type>(rm, result.value);
        return ctx.end_next();
    }

    // ROT_OP rm, CL
    template <byte_t OP> requires(match8("1101001w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [rot, rm] = Decode::opt_rm(ctx);
        auto const lhs = ctx.rm_get<type>(rm);
        auto const rhs = ctx.reg_get<byte_t>(REG::CL);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_rot[rot](flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<type>(rm, result.value);
        return ctx.end_next();
    }

    // MISC1_OP rm
    template <byte_t OP> requires(match8("1111011w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [misc1, rm] = Decode::opt_rm(ctx);
        return MISC<type>::table_misc1[misc1](ctx, rm);
    }

    // MISC2_OP rm
    template <byte_t OP> requires(match8("1111111w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [misc2, rm] = Decode::opt_rm(ctx);
        return MISC<type>::table_misc2[misc2](ctx, rm);
    }

    // ROT_OP rm, imm
    template <byte_t OP> requires(match8("1100000w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        auto const [rot, rm] = Decode::opt_rm(ctx);
        auto const imm = Decode::imm<byte_t>(ctx);
        auto const lhs = ctx.rm_get<type>(rm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<type>::table_rot[rot](flags, lhs, imm);
        ctx.flags_set<Flags>(result.flags);
        ctx.rm_set<type>(rm, result.value);
        return ctx.end_next();
    }

    // ENTER imm16, imm8
    template <byte_t OP> requires(match8("11001000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const size = Decode::imm<word_t>(ctx);
        auto const level = Decode::imm<byte_t>(ctx);
        ctx.push_frame_local(size, level);
        return ctx.end_next();
    }

    // LEAVE
    template <byte_t OP> requires(match8("11001001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        ctx.pop_frame_local();
        return ctx.end_next();
    }

    // PUSH imW
    template <byte_t OP> requires(match8("011010w0", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<!(OP & 0b10)>;
        auto const imm = Decode::imm<type>(ctx);
        ctx.push<type>(imm);
        return ctx.end_next();
    }

    // IMUL imW
    template <byte_t OP> requires(match8("011010w1", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<!(OP & 0b10)>;
        auto const [reg, rm] = Decode::reg_rm(ctx);
        auto const imm = Decode::imm<type>(ctx);
        auto const lhs = to_signed(ctx.rm_get<word_t>(rm));
        auto const rhs = to_signed(imm);
        auto const flags = ctx.flags_get<Flags>();
        auto const result = ALU<word_t>::op_imul(flags, lhs, rhs);
        ctx.flags_set<Flags>(result.flags);
        ctx.reg_set<word_t>(reg, result.value);
        return ctx.end_bad();
    }

    // INS
    template <byte_t OP> requires(match8("0110110w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const port = ctx.reg_get<word_t>(REG::DX);
            auto const addr_dst = ctx.str_dst<type>();
            auto const value = ctx.port_get<type>(port);
            ctx.mem_set<type>(addr_dst, value);
            return true;
        });
    }

    // OUTS
    template <byte_t OP> requires(match8("0110111w", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        using type = Decode::type<OP & 0b1>;
        return ctx.end_repeat([](CTX ctx) -> bool {
            auto const port = ctx.reg_get<word_t>(REG::DX);
            auto const addr_src = ctx.str_src<type>();
            auto const value = ctx.mem_get<type>(addr_src);
            ctx.port_set<type>(port, value);
            return true;
        });
    }

    // PUSHA
    template <byte_t OP> requires(match8("01100000", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const ax = ctx.reg_get<word_t>(REG::AX);
        auto const cx = ctx.reg_get<word_t>(REG::CX);
        auto const dx = ctx.reg_get<word_t>(REG::DX);
        auto const bx = ctx.reg_get<word_t>(REG::BX);
        auto const sp = ctx.reg_get<word_t>(REG::SP);
        auto const bp = ctx.reg_get<word_t>(REG::BP);
        auto const si = ctx.reg_get<word_t>(REG::SI);
        auto const di = ctx.reg_get<word_t>(REG::DI);
        ctx.push<word_t>(ax);
        ctx.push<word_t>(cx);
        ctx.push<word_t>(dx);
        ctx.push<word_t>(bx);
        ctx.push<word_t>(sp);
        ctx.push<word_t>(bp);
        ctx.push<word_t>(si);
        ctx.push<word_t>(di);
        return ctx.end_next();
    }

    // POPA
    template <byte_t OP> requires(match8("01100001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const di = ctx.pop<word_t>();
        auto const si = ctx.pop<word_t>();
        auto const bp = ctx.pop<word_t>();
        auto const sp = ctx.pop<word_t>();
        auto const bx = ctx.pop<word_t>();
        auto const dx = ctx.pop<word_t>();
        auto const cx = ctx.pop<word_t>();
        auto const ax = ctx.pop<word_t>();
        ctx.reg_set<word_t>(REG::AX, ax);
        ctx.reg_set<word_t>(REG::CX, cx);
        ctx.reg_set<word_t>(REG::DX, dx);
        ctx.reg_set<word_t>(REG::BX, bx);
        ctx.reg_set<word_t>(REG::SP, sp); // NOTE: technically this should be skiped
        ctx.reg_set<word_t>(REG::BP, bp);
        ctx.reg_set<word_t>(REG::SI, si);
        ctx.reg_set<word_t>(REG::DI, di);
        return ctx.end_next();
    }

    // BOUND
    template <byte_t OP> requires(match8("01100010", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        auto const [reg, rm] = Decode::reg_rm(ctx);
        if (rm.is_reg) {
            return ctx.end_bad();
        }
        auto const value = ctx.reg_get<word_t>(reg);
        auto const [lo, up] = ctx.rm_get<FAR>(rm);
        if (value >= lo && value <= up) {
            return ctx.end_next();
        }
        ctx.push_frame_interupt();
        return ctx.end_interupt(5);
    }

    // RESERVED63
    template <byte_t OP> requires(match8("01100011", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_bad();
    }

    // RESERVED64-67
    template <byte_t OP> requires(match8("011001xx", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_bad();
    }

    // ResultF1
    template <byte_t OP> requires(match8("11110001", OP))
    [[nodiscard]] static constexpr Result op(CTX ctx) noexcept {
        return ctx.end_bad();
    }

    struct OPTable {
        Result(* const ops[256])(CTX) noexcept;
    };
    static constexpr auto const table = []<std::size_t...OP>(std::index_sequence<OP...>) consteval {
        return OPTable {  &op<OP>... };
    } (std::make_index_sequence<256>());
#pragma clang diagnostic pop
};

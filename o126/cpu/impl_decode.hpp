#pragma once
#include "impl.hpp"
#include "impl_ctx.hpp"

struct o126::CPU::IMPL::Decode final {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
    template <bool is_word>
    using type = std::conditional_t<is_word, word_t, byte_t>;

    template<SEG S, REG B = REG::NONE, REG I = REG::NONE, typename D = void>
    static constexpr FAR sbi_impl(CTX ctx) noexcept {
        auto disp = word_t{};
        if constexpr (B != REG::NONE) {
            disp += ctx.reg_get<word_t>(B);
        }
        if constexpr (I != REG::NONE) {
            disp += ctx.reg_get<word_t>(I);
        }
        if constexpr (std::is_same_v<D, byte_t>) {
            disp += to_signed(ctx.fetch<byte_t>());
        } else if constexpr (std::is_same_v<D, word_t>) {
            disp += to_signed(ctx.fetch<word_t>());
        }
        auto const seg = ctx.seg_get(S);
        return { disp, seg };
    }

    static constexpr FAR(* const mod_table[3][8])(CTX) noexcept = {
        {
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::SI, void>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::DI, void>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::SI, void>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::DI, void>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::SI, void>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::DI, void>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::NONE, word_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::NONE, void>,
        },
        {
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::SI, byte_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::DI, byte_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::SI, byte_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::DI, byte_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::SI, byte_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::DI, byte_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::NONE, byte_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::NONE, byte_t>,
        },
        {
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::SI, word_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::DI, word_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::SI, word_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::DI, word_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::SI, word_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::NONE, REG::DI, word_t>,
            &sbi_impl<SEG::SS_OR_PREFIX, REG::BP, REG::NONE, word_t>,
            &sbi_impl<SEG::DS_OR_PREFIX, REG::BX, REG::NONE, word_t>,
        },
    };

    template<std::same_as<byte_t> T>
    [[nodiscard]] static constexpr byte_t imm(CTX ctx) noexcept {
        auto const result = ctx.fetch<byte_t>();
        return result;
    }

    template<std::same_as<word_t> T>
    [[nodiscard]] static constexpr word_t imm(CTX ctx) noexcept {
        auto const result = ctx.fetch<word_t>();
        return result;
    }

    template<std::same_as<byte_t> T>
    [[nodiscard]] static constexpr sbyte_t rel(CTX ctx) noexcept {
        auto const result = ctx.fetch<byte_t>();
        return static_cast<sbyte_t>(result);
    }

    template<std::same_as<word_t> T>
    [[nodiscard]] static constexpr sword_t rel(CTX ctx) noexcept {
        auto const result = ctx.fetch<word_t>();
        return static_cast<sword_t>(result);
    }

    template<std::same_as<FAR> T>
    [[nodiscard]] static constexpr FAR addr(CTX ctx) noexcept {
        auto const disp = ctx.fetch<word_t>();
        auto const seg = ctx.fetch<word_t>();
        return { disp, seg };
    }

    [[nodiscard]] static constexpr Opt_RM opt_rm(CTX ctx) noexcept {
        auto const modregrm = ctx.fetch<byte_t>();
        auto const mod = modregrm >> 6;
        auto const opt = static_cast<byte_t>((modregrm >> 3) & 7);
        auto const reg = modregrm & 7;
        if (mod == 0b11) {
            return { opt, { .is_reg = true, .reg = static_cast<REG>(reg) } };
        } else {
            return { opt, { .is_reg = false, .ptr = mod_table[mod][reg](ctx) } };
        }
    }

    [[nodiscard]] static constexpr REG_RM reg_rm(CTX ctx) noexcept {
        auto const [opt, rm] = Decode::opt_rm(ctx);
        return { static_cast<REG>(opt), rm };
    }

    [[nodiscard]] static constexpr SEG_RM seg_rm(CTX ctx) noexcept {
        auto const [opt, rm] = Decode::opt_rm(ctx);
        return { static_cast<SEG>(opt & 3), rm };
    }
#pragma clang diagnostic pop
};

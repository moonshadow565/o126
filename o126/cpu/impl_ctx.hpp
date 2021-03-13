#pragma once
#include "impl.hpp"
#include <concepts>
#include <utility>

struct o126::CPU::IMPL::CTX final {
    CPU& cpu;
    BUS& bus;

    /// Flags
    template <std::same_as<Flags> T>
    [[nodiscard]] constexpr Flags flags_get() const noexcept {
        return cpu.flags;
    }

    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t flags_get() const noexcept {
        auto const flags = flags_get<Flags>();
        byte_t result = {};
        result |= flags.carry << 0;
        result |= 1 << 1;
        result |= flags.parity << 2;
        result |= flags.auxiliary << 4;
        result |= flags.zero << 6;
        result |= flags.sign << 7;
        return result;
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t flags_get() const noexcept {
        auto const flags = flags_get<Flags>();
        word_t result = {};
        result |= flags.carry << 0;
        result |= 1 << 1;
        result |= flags.parity << 2;
        result |= flags.auxiliary << 4;
        result |= flags.zero << 6;
        result |= flags.sign << 7;
        result |= flags.trap << 8;
        result |= flags.interupt << 9;
        result |= flags.direction << 10;
        result |= flags.overflow << 11;
        return result;
    }

    template <std::same_as<Flags> T>
    constexpr void flags_set(Flags value) const noexcept {
        cpu.flags = value;
    }

    template <std::same_as<byte_t> T>
    constexpr void flags_set(byte_t value) const noexcept {
        auto flags = flags_get<Flags>();
        flags.carry = value & (1 << 0);
        flags.parity = value & (1 << 2);
        flags.auxiliary = value & (1 << 4);
        flags.zero = value & (1 << 6);
        flags.sign = value & (1 << 7);
        flags_set<Flags>(flags);
    }

    template <std::same_as<word_t> T>
    constexpr void flags_set(word_t value) const noexcept {
        auto flags = flags_get<Flags>();
        flags.carry = value & (1 << 0);
        flags.parity = value & (1 << 2);
        flags.auxiliary = value & (1 << 4);
        flags.zero = value & (1 << 6);
        flags.sign = value & (1 << 7);
        flags.trap = value & (1 << 8);
        flags.interupt = value & (1 << 9);
        flags.direction = value & (1 << 10);
        flags.overflow = value & (1 << 11);
        flags_set<Flags>(flags);
    }

    /// Segment read/write
    [[nodiscard]] constexpr word_t seg_get(SEG seg) const noexcept {
        if (static_cast<int>(seg) & 4) {
            if (auto const seg_override = cpu.prefix.seg; seg_override != SEG::NONE) {
                seg = seg_override;
            }
        }
        return cpu.segs[static_cast<int>(seg) & 3];
    }

    constexpr void seg_set(SEG seg, word_t val) const noexcept {
        if (static_cast<int>(seg) & 4) {
            if (auto const seg_override = cpu.prefix.seg; seg_override != SEG::NONE) {
                seg = seg_override;
            }
        }
        cpu.segs[static_cast<int>(seg)] = val;
    }

    /// Register read/write
    template<std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t reg_get(REG reg) const noexcept {
        if (static_cast<int>(reg) & 4) {
            return static_cast<byte_t>(cpu.regs[static_cast<int>(reg) & 3] >> 8);
        } else {
            return static_cast<byte_t>(cpu.regs[static_cast<int>(reg) & 3]);
        }
    }

    template<std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t reg_get(REG reg) const noexcept {
        return cpu.regs[static_cast<int>(reg)];
    }

    template<std::same_as<byte_t> T>
    constexpr void reg_set(REG reg, byte_t val) const noexcept {
        auto& result = cpu.regs[static_cast<int>(reg) & 3];
        if (static_cast<int>(reg) & 4) {
            result = static_cast<word_t>((result & 0x00FF) | (val << 8));
        } else {
            result = static_cast<word_t>((result & 0xFF00) | val);
        }
    }

    template<std::same_as<word_t> T>
    constexpr void reg_set(REG reg, word_t val) const noexcept {
        cpu.regs[static_cast<int>(reg)] = val;
    }

    constexpr void reg_add(REG reg, sword_t diff) const noexcept {
        cpu.regs[static_cast<int>(reg)] += diff;
    }

    /// Accumulator pair set

    template<std::same_as<byte_t> T>
    constexpr auto pair_get() const noexcept {
        struct PAIR {
            byte_t lo = {};
            byte_t hi = {};
        };
        return PAIR { reg_get<byte_t>(REG::AL), reg_get<byte_t>(REG::AH) };
    }

    template<std::same_as<word_t> T>
    constexpr auto pair_get() const noexcept {
        struct PAIR {
            word_t lo = {};
            word_t hi = {};
        };
        return PAIR { reg_get<word_t>(REG::AX), reg_get<word_t>(REG::DX) };
    }

    template<std::same_as<byte_t> T>
    constexpr void pair_set(byte_t lo, byte_t hi) const noexcept {
        reg_set<byte_t>(REG::AL, lo);
        reg_set<byte_t>(REG::AH, hi);
    }

    template<std::same_as<word_t> T>
    constexpr void pair_set(word_t lo, word_t hi) const noexcept {
        reg_set<word_t>(REG::AX, lo);
        reg_set<word_t>(REG::DX, hi);
    }

    /// Register pre increment and post increment
    [[nodiscard]] constexpr FAR ptr_get(REG reg, SEG seg) const noexcept {
        return { reg_get<word_t>(reg), seg_get(seg) };
    }

    constexpr void ptr_set(REG reg, SEG seg, FAR addr) const noexcept {
        reg_set<word_t>(reg, addr.disp);
        seg_set(seg, addr.seg);
    }

    [[nodiscard]] constexpr FAR ptr_inc_post(REG reg, SEG seg, sword_t diff) const noexcept {
        auto const result = ptr_get(reg, seg);
        reg_add(reg, diff);
        return result;
    }

    [[nodiscard]] constexpr FAR ptr_inc_pre(REG reg, SEG seg, sword_t diff) const noexcept {
        reg_add(reg, diff);
        return ptr_get(reg, seg);
    }

    /// Port read/write
    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t port_get(word_t port) const noexcept {
        return bus.in_byte(port);
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t port_get(word_t port) const noexcept {
        return bus.in_word(port);
    }

    template <std::same_as<byte_t> T>
    constexpr void port_set(word_t port, byte_t val) const noexcept {
        bus.out_byte(port, val);
    }

    template <std::same_as<word_t> T>
    constexpr void port_set(word_t port, word_t val) const noexcept {
        bus.out_word(port, val);
    }

    /// Memory read/write
    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t mem_get(FAR addr) const noexcept {
        return bus.read_byte(addr);
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t mem_get(FAR addr) const noexcept {
        return bus.read_word(addr);
    }

    template <std::same_as<FAR> T>
    [[nodiscard]] constexpr FAR mem_get(FAR addr) const noexcept {
        auto const disp = mem_get<word_t>(addr);
        auto const seg = mem_get<word_t>(addr + 2);
        return { disp, seg };
    }

    template <std::same_as<byte_t> T>
    constexpr void mem_set(FAR addr, byte_t val) const noexcept {
        bus.write_byte(addr, val);
    }

    template <std::same_as<word_t> T>
    constexpr void mem_set(FAR addr, word_t val) const noexcept {
        bus.write_word(addr, val);
    }

    template <std::same_as<FAR> T>
    constexpr void mem_set(FAR addr, FAR val) const noexcept {
        mem_set<word_t>(addr, val.disp);
        mem_set<word_t>(addr + 2, val.seg);
    }

    /// Register/Memory read/write
    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t rm_get(RM rm) const noexcept {
        if (rm.is_reg) {
            return reg_get<T>(rm.reg);
        } else {
            return mem_get<T>(rm.ptr);
        }
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t rm_get(RM rm) const noexcept {
        if (rm.is_reg) {
            return reg_get<T>(rm.reg);
        } else {
            return mem_get<T>(rm.ptr);
        }
    }

    template <std::same_as<FAR> T>
    [[nodiscard]] constexpr FAR rm_get(RM rm) const noexcept {
        return mem_get<T>(rm.ptr);
    }

    template <std::same_as<byte_t> T>
    constexpr void rm_set(RM rm, byte_t val) const noexcept {
        if (rm.is_reg) {
            reg_set<T>(rm.reg, val);
        } else {
            mem_set<T>(rm.ptr, val);
        }
    }

    template <std::same_as<word_t> T>
    constexpr void rm_set(RM rm, word_t val) const noexcept {
        if (rm.is_reg) {
            reg_set<T>(rm.reg, val);
        } else {
            mem_set<T>(rm.ptr, val);
        }
    }

    template <std::same_as<FAR> T>
    constexpr void rm_set(RM rm, FAR val) const noexcept {
        mem_set<T>(rm.ptr, val);
    }

    /// Stack
    template<std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t pop() const noexcept {
        auto const addr = ptr_inc_post(REG::SP, SEG::SS, 1);
        return mem_get<byte_t>(addr);
    }

    template<std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t pop() const noexcept {
        auto const addr = ptr_inc_post(REG::SP, SEG::SS, 2);
        return mem_get<word_t>(addr);
    }

    template <std::same_as<byte_t> T>
    constexpr void push(byte_t value) const noexcept {
        auto const addr = ptr_inc_pre(REG::SP, SEG::SS, -1);
        mem_set<byte_t>(addr, value);
    }

    template <std::same_as<word_t> T>
    constexpr void push(word_t value) const noexcept {
        auto const addr = ptr_inc_pre(REG::SP, SEG::SS, -2);
        mem_set<word_t>(addr, value);
    }

    /// Stack frame
    constexpr void pop_frame_local() const noexcept {
        auto const next_sp = reg_get<word_t>(REG::BP);
        reg_set<word_t>(REG::SP, next_sp);
        auto const next_base = pop<word_t>();
        reg_set<word_t>(REG::BP, next_base);
    }

    [[nodiscard]] constexpr word_t pop_frame_near() const noexcept {
        return pop<word_t>();
    }

    [[nodiscard]] constexpr FAR pop_frame_far() const noexcept {
        auto const disp = pop<word_t>();
        auto const seg = pop<word_t>();
        return { disp, seg };
    }

    [[nodiscard]] constexpr FAR pop_frame_interupt() const noexcept {
        auto const disp = pop<word_t>();
        auto const seg = pop<word_t>();
        auto const flags = pop<word_t>();
        flags_set<word_t>(flags);
        return { disp, seg };
    }

    constexpr void push_frame_local(word_t size, byte_t level) const noexcept {
        auto const old_base = reg_get<word_t>(REG::BP);
        push<word_t>(old_base);
        auto const old_sp = reg_get<word_t>(REG::SP);
        if (level %= 32) {
            for (auto i = 1; i < level; ++i) {
                auto const addr = ptr_inc_pre(REG::BP, SEG::SS, -2);
                auto const value = mem_get<word_t>(addr);
                push<word_t>(value);
            }
            push<word_t>(old_sp);
        }
        reg_set<word_t>(REG::BP, old_sp);
        reg_add(REG::SP, static_cast<sword_t>(-size));
    }

    constexpr void push_frame_near() const noexcept {
        auto const addr_cur = reg_get<word_t>(REG::IP);
        push<word_t>(addr_cur);
    }

    constexpr void push_frame_far() const noexcept {
        auto const addr_cur = ptr_get(REG::IP, SEG::CS);
        push<word_t>(addr_cur.seg);
        push<word_t>(addr_cur.disp);
    }

    constexpr void push_frame_interupt() const noexcept {
        auto const flags = flags_get<word_t>();
        push<word_t>(flags);
        auto const addr_cur = ptr_get(REG::IP, SEG::CS);
        push<word_t>(addr_cur.seg);
        push<word_t>(addr_cur.disp);
    }

    constexpr void inst_repeat() const noexcept {
        reg_add(REG::IP, -cpu.inst_len);
    }

    /// String operations addressing
    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr FAR str_src() const noexcept {
        auto const flags = flags_get<Flags>();
        auto const diff = static_cast<sword_t>(flags.direction ? -1 : 1);
        auto const addr = ptr_inc_post(REG::SI, SEG::DS_OR_PREFIX, diff);
        return addr;
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr FAR str_src() const noexcept {
        auto const flags = flags_get<Flags>();
        auto const diff = static_cast<sword_t>(flags.direction ? -2 : 2);
        auto const addr = ptr_inc_post(REG::SI, SEG::DS_OR_PREFIX, diff);
        return addr;
    }

    template <std::same_as<byte_t> T>
    [[nodiscard]] constexpr FAR str_dst() const noexcept {
        auto const flags = flags_get<Flags>();
        auto const diff = static_cast<sword_t>(flags.direction ? -1 : 1);
        auto const addr = ptr_inc_post(REG::DI, SEG::ES, diff);
        return addr;
    }

    template <std::same_as<word_t> T>
    [[nodiscard]] constexpr FAR str_dst() const noexcept {
        auto const flags = flags_get<Flags>();
        auto const diff = static_cast<sword_t>(flags.direction ? -2 : 2);
        auto const addr = ptr_inc_post(REG::DI, SEG::ES, diff);
        return addr;
    }

    [[nodiscard]] constexpr bool str_rep() const noexcept {
        auto const flags = flags_get<Flags>();
        switch (cpu.prefix.rep) {
        case REP::NOT_ZERO:
            return !flags.zero;
        case REP::ZERO:
            return flags.zero;
        default:
            return false;
        }
    }

    /// Fetch instruction
    template<std::same_as<byte_t> T>
    [[nodiscard]] constexpr byte_t fetch() const noexcept {
        cpu.inst_len += 1;
        auto const addr = ptr_inc_post(REG::IP, SEG::CS, 1);
        auto const result = mem_get<byte_t>(addr);
        return result;
    }

    template<std::same_as<word_t> T>
    [[nodiscard]] constexpr word_t fetch() const noexcept {
        cpu.inst_len += 2;
        auto const addr = ptr_inc_post(REG::IP, SEG::CS, 2);
        auto const result = mem_get<word_t>(addr);
        return result;
    }

    /// End instruction
    [[nodiscard]] constexpr Result end_bad() const noexcept {
        cpu.prefix = {};
        cpu.inst_len = {};
        push_frame_interupt();
        return end_interupt(6);
    }

    [[nodiscard]] constexpr Result end_prefix_seg(SEG seg) const noexcept {
        if (cpu.inst_len >= 15) {
            return end_bad();
        }
        cpu.prefix.seg = seg;
        return Result::PREFIX;
    }

    [[nodiscard]] constexpr Result end_prefix_rep(REP rep) const noexcept {
        if (cpu.inst_len >= 15) {
            return end_bad();
        }
        cpu.prefix.rep = rep;
        return Result::PREFIX;
    }

    [[nodiscard]] constexpr Result end_prefix_lock() const noexcept {
        if (cpu.inst_len >= 15) {
            return end_bad();
        }
        cpu.prefix.lock = true;
        return Result::PREFIX;
    }

    [[nodiscard]] constexpr Result end_next() const noexcept {
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::DONE;
    }

    [[nodiscard]] constexpr Result end_halt() const noexcept {
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::HALT;
    }

    [[nodiscard]] constexpr Result end_wait() const noexcept {
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::WAIT;
    }

    [[nodiscard]] constexpr Result end_jmp_rel(sword_t diff) const noexcept {
        reg_add(REG::IP, diff);
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::DONE;
    }

    [[nodiscard]] constexpr Result end_jmp_near(word_t addr) const noexcept {
        reg_set<word_t>(REG::IP, addr);
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::DONE;
    }

    [[nodiscard]] constexpr Result end_jmp_far(FAR addr) const noexcept {
        ptr_set(REG::IP, SEG::CS, addr);
        cpu.prefix = {};
        cpu.inst_len = {};
        return Result::DONE;
    }

    [[nodiscard]] constexpr Result end_interupt(byte_t index) const noexcept {
        auto flags = flags_get<Flags>();
        flags.interupt = false;
        flags.trap = false;
        flags_set<Flags>(flags);

        auto const addr_next_offset = FAR { static_cast<word_t>(index * 4),  0 };
        auto const addr_next = mem_get<FAR>(addr_next_offset);
        return end_jmp_far(addr_next);
    }

    template <typename F>
    [[nodiscard]] constexpr Result end_repeat(F&& func) const noexcept {
        if (cpu.prefix.rep == REP::NONE) {
            std::forward<F>(func)(*this);
            return end_next();
        }
        auto count = reg_get<word_t>(REG::CX);
        while (count != 0) {
            --count;
            if (!std::forward<F>(func)(*this)) {
                break;
            }
        }
        reg_set<word_t>(REG::CX, count);
        return end_next();
    }
};

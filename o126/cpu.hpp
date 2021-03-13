#ifndef O126_CPU_HPP
#define O126_CPU_HPP
#include <cinttypes>

namespace o126 {
using byte_t = std::uint8_t;
using word_t = std::uint16_t;
using dword_t = std::uint32_t;
using sbyte_t = std::int8_t;
using sword_t = std::int16_t;
using sdword_t = std::int32_t;

struct CPU final {
public:
    enum class Result {
        DONE,
        PREFIX,
        HALT,
        WAIT,
    };

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

    struct BUS {
        constexpr virtual byte_t read_byte(FAR addr) noexcept = 0;
        constexpr virtual void write_byte(FAR addr, byte_t val) noexcept = 0;
        constexpr virtual word_t read_word(FAR addr) noexcept = 0;
        constexpr virtual void write_word(FAR addr, word_t val) noexcept = 0;

        constexpr virtual byte_t in_byte(word_t port) noexcept = 0;
        constexpr virtual void out_byte(word_t port, byte_t val) noexcept = 0;
        constexpr virtual word_t in_word(word_t port) noexcept = 0;
        constexpr virtual void out_word(word_t port, word_t val) noexcept = 0;
    };
private:
    enum class REG : sbyte_t {
        NONE = - 1,
        AX = 0, // Accumulator
        CX = 1, // Counter
        DX = 2, // Data
        BX = 3, // Base
        SP = 4, // Stack pointer
        BP = 5, // Base pointer
        SI = 6, // Source index
        DI = 7, // Destination index
        IP = 8, // Instruction pointer
        AL = 0,
        CL = 1,
        DL = 2,
        BL = 3,
        AH = 4,
        CH = 5,
        DH = 6,
        BH = 7,
        COUNT = 9,
    };

    enum class SEG : sbyte_t {
        NONE = - 1,
        ES = 0, // Extra segment
        CS = 1, // Code segment
        SS = 2, // Stack segment
        DS = 3, // Data segment
        ES_OR_PREFIX = 4,
        CS_OR_PREFIX = 5,
        SS_OR_PREFIX = 6,
        DS_OR_PREFIX = 7,
        COUNT = 4,
    };

    enum class REP : sbyte_t {
        NONE = -1,
        NOT_ZERO = 0,
        ZERO = 1,
    };

    struct Flags {
        bool carry : 1 = {};        // 0    1           0x0001
        bool reserved1 : 1 = {};    // 1    2           0x0002
        bool parity : 1 = {};       // 2    4           0x0004
        bool reserved3 : 1 = {};    // 3    8           0x0008
        bool auxiliary : 1 = {};    // 4    16          0x0010
        bool reserved5 : 1 = {};    // 5    32          0x0020
        bool zero : 1 = {};         // 6    64          0x0040
        bool sign : 1 = {};         // 7    128         0x0080
        bool trap : 1 = {};         // 8    256         0x0100
        bool interupt : 1 = {};     // 9    512         0x0200
        bool direction : 1 = {};    // 10   1024        0x0400
        bool overflow : 1 = {};     // 11   2048        0x0800
        bool reserved12 : 1 = {};   // 12   4096        0x1000
        bool reserved13 : 1 = {};   // 13   8192        0x2000
        bool reserved14 : 1 = {};   // 14   16384       0x4000
        bool reserved15 : 1 = {};   // 15   32768       0x8000
    };

    struct Prefix final {
        bool lock = {};
        SEG seg = SEG::NONE;
        REP rep = REP::NONE;
    };

    word_t regs[static_cast<int>(REG::COUNT)] = { 0, 0, 0, 0, 0, 0, 0, 0, 0xFFF0 };
    word_t segs[static_cast<int>(SEG::COUNT)] = { 0, 0xF000, 0, 0 };
    Prefix prefix = {};
    std::uint8_t inst_len = {};
    Flags flags = {};

    struct IMPL;
public:
    Result exec(BUS& bus) noexcept;
};
}


#endif // O126_HPP
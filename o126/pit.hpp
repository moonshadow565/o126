#ifndef O126_PIT_HPP
#define O126_PIT_HPP
#include "common.hpp"

struct o126::PIT {
private:
    enum class Mode : sbyte_t {
        NONE = -1,
        INTERUPT_TERMINAL = 0b000,
        HW_ONESHOT = 0b001,
        RATE = 0b010,
        RATE2 = 0b110,
        SQUARE_WAVE = 0b011,
        SQUARE_WAVE2 = 0b111,
        SW_STROBE = 0b100,
        HW_STROBE = 0b101,
    };

    enum class Latch : sbyte_t {
        NONE = 0b00,
        LO = 0b01,
        HI = 0b10,
        LO_HI = 0b11,
    };

    struct Channel {
        word_t count = {};
        word_t input_latch = {};
        word_t output_latch = {};
        bool output_latch_enable = {};
        bool input_latch_enable = {};
        Latch latch = {};
        Mode mode = Mode::NONE;
    };
    Channel channels[3] = {};
public:
    constexpr void set_command(byte_t command) noexcept {
    }

    constexpr byte_t get_counter(byte_t index) noexcept {
        return 0;
    }

    constexpr void set_counter(byte_t index, byte_t value) noexcept {
    }

    constexpr void step(byte_t index) noexcept {
        auto& channel = channels[index];
        switch (channel.mode) {
        case Mode::NONE:
            break;
        case Mode::INTERUPT_TERMINAL: // Mode 0: interupt on terminal count
            break;
        case Mode::HW_ONESHOT: // Mode 1: hardware retriggerable one-shot
            break;
        case Mode::RATE: // Mode 2: rate generator
        case Mode::RATE2:
            break;
        case Mode::SQUARE_WAVE: // Mode 3: square wave generator
        case Mode::SQUARE_WAVE2:
            break;
        case Mode::SW_STROBE: // Mode 4: software triggered strobe
            break;
        case Mode::HW_STROBE: // Mode 5: hardware retriggerable strobe
            break;
        }
    }
};

#endif // O126_PIT_HPP

#ifndef O126_BUS_HPP
#define O126_BUS_HPP
#include "common.hpp"

struct o126::BUS {
    constexpr BUS() noexcept = default;
    BUS(BUS const&) = delete;
    BUS(BUS&&) = delete;
    BUS& operator=(BUS const&) = delete;
    BUS& operator=(BUS&&) = delete;
    constexpr virtual byte_t read_byte(FAR addr) noexcept = 0;
    constexpr virtual void write_byte(FAR addr, byte_t val) noexcept = 0;
    constexpr virtual word_t read_word(FAR addr) noexcept = 0;
    constexpr virtual void write_word(FAR addr, word_t val) noexcept = 0;

    constexpr virtual byte_t in_byte(word_t port) noexcept = 0;
    constexpr virtual void out_byte(word_t port, byte_t val) noexcept = 0;
    constexpr virtual word_t in_word(word_t port) noexcept = 0;
    constexpr virtual void out_word(word_t port, word_t val) noexcept = 0;
};
#endif // O126_BUS_HPP

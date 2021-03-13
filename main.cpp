#include <iostream>
#include <array>
#include <filesystem>
#include <fstream>
#include <vector>
#include <type_traits>
#include <utility>
#include "o126/cpu.hpp"

using namespace o126;

struct MEM : CPU::BUS {
    using FAR = CPU::FAR;
    std::array<byte_t, 0x10'00'00> data = {};

    constexpr MEM() noexcept = default;

    void load_bios(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw "Failed to open BIOS file!";
        }
        auto const data_size =  static_cast<std::ptrdiff_t>(data.size());
        auto const file_size = static_cast<std::ptrdiff_t>(std::filesystem::file_size(filename));
        auto const offset = data_size - file_size;
        if (offset < 0 || offset > data_size) {
            throw "BIOS file too big!";
        }
        file.read(reinterpret_cast<char*>(data.data() + offset), file_size);
    }

    constexpr virtual byte_t read_byte(FAR addr) noexcept override {
        auto const lo = data[addr.ea()];
        return lo;
    }
    constexpr virtual void write_byte(FAR addr, byte_t val) noexcept override {
        data[addr.ea()] = val;
    }
    constexpr virtual word_t read_word(FAR addr) noexcept override {
        auto const lo = data[addr.ea()];
        auto const hi = data[(addr + 1).ea()];
        return static_cast<word_t>(lo | (hi << 8));
    }
    constexpr virtual void write_word(FAR addr, word_t val) noexcept override {
        data[addr.ea()] = static_cast<byte_t>(val);
        data[(addr + 1).ea()] = static_cast<byte_t>(val >> 8);
    }

    constexpr virtual byte_t in_byte(word_t port) noexcept override {
        (void)port;
        return {};
    }
    constexpr virtual void out_byte(word_t port, byte_t val) noexcept override {
        (void)port;
        (void)val;
    }
    constexpr virtual word_t in_word(word_t port) noexcept override {
        (void)port;
        return {};
    }
    constexpr virtual void out_word(word_t port, word_t val) noexcept override {
        (void)port;
        (void)val;
    }
};

void test_inst(std::string name) {
    printf("Testing %s:\n", name.c_str());
    auto mem = MEM();
    mem.load_bios("80186_tests/"+name+".bin");
    auto cpu = CPU{};
    for (;;) {
        auto const result = cpu.exec(mem);
        if (result == CPU::Result::HALT) {
            break;
        }
    }

    std::ifstream file("80186_tests/res_"+name+".bin", std::ios::binary);
    if (!file) {
        throw "Failed to open verification file!";
    }
    byte_t c = 0;
    for (word_t i = 0; file.read(reinterpret_cast<char*>(&c), 1); i += 1) {
        byte_t c2 = mem.data[i];
        if (c2 != c) {
            printf("Bad (%d): %02X should be %02X\n", i, c2, c);
        }
    }
}

int main() {
    test_inst("add");
    test_inst("sub");
    test_inst("jump1");
    test_inst("jump2");
    test_inst("strings");
    test_inst("interrupt");
    test_inst("cmpneg");
    test_inst("bitwise");
    test_inst("rep");
    test_inst("rotate");
    test_inst("control");
    test_inst("mul");
    test_inst("div");
    test_inst("segpr");
    test_inst("bcdcnv");
    test_inst("shifts");

//    test_inst("datatrnf"); // broken test ??

    return 0;
}

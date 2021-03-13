#include <cstdio>
#include "cpu/impl_exe.hpp"

o126::CPU::Result o126::CPU::exec(BUS &bus) noexcept {
    auto ctx = IMPL::CTX { *this, bus };
    // auto const ip = ctx.ptr_get(REG::IP, SEG::CS);
    for (;;) {
        auto const op = ctx.fetch<byte_t>();
        auto const result = IMPL::EXE::table.ops[op](ctx);
        switch(result) {
        case Result::PREFIX:
            continue;
        case Result::HALT:
        case Result::WAIT:
        case Result::DONE:
            if (auto const flags = ctx.flags_get<Flags>(); flags.trap) {
                ctx.push_frame_interupt();
                (void)ctx.end_interupt(1);
            }
            return result;
        }
    }
}



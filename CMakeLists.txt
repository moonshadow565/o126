cmake_minimum_required(VERSION 3.5)

project(o126 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wold-style-cast -Wnarrowing -Wno-unknown-pragmas")

add_executable(o126
    o126/cpu.cpp
    o126/cpu.hpp
    o126/cpu/impl.hpp
    o126/cpu/impl_alu.hpp
    o126/cpu/impl_bcd.hpp
    o126/cpu/impl_ctx.hpp
    o126/cpu/impl_decode.hpp
    o126/cpu/impl_exe.hpp
    o126/cpu/impl_misc.hpp
    main.cpp)

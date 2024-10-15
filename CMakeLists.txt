cmake_minimum_required(VERSION 3.21)

project(embers_project)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(embers)

add_subdirectory(sandbox)

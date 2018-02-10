cmake_minimum_required(VERSION 2.7)

if(TARGET abacus)
    return()
endif(TARGET abacus)

project(libabacus)

add_compile_options(-pedantic -Wall)

add_executable(libabacus src/main.c)

set_property(TARGET libabacus PROPERTY C_STANDARD 90)
target_include_directories(libabacus PUBLIC include)

cmake_minimum_required(VERSION 2.7)

if(TARGET abacus)
    return()
endif(TARGET abacus)

project(libabacus)

add_compile_options(-pedantic -Wall)

add_library(abacus STATIC src/lexer.c src/util.c src/table.c src/parser.c src/libabacus.c src/tree.c src/debug.c src/parsetype.c src/reserved.c src/trie.c src/refcount.c src/ref_vec.c src/ref_trie.c src/basetype.c src/value.c src/custom.c src/interpreter.c src/function_list.c)
add_executable(libabacus src/main.c)
add_subdirectory(external/liblex)

set_property(TARGET abacus PROPERTY C_STANDARD 90)
set_property(TARGET libabacus PROPERTY C_STANDARD 90)
target_include_directories(abacus PUBLIC include)
target_include_directories(libabacus PUBLIC include)

target_link_libraries(abacus lex)
target_link_libraries(libabacus abacus)

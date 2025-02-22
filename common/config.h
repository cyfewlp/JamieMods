//
// Created by jamie on 2025/2/22.
//

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#ifdef __clang__
    // Declare a LIBC_NAMESPACE with hidden visibility. `namespace
    // LIBC_NAMESPACE_DECL {` should be used around all declarations and definitions
    // for libc internals as opposed to just `namespace LIBC_NAMESPACE {`. This
    // ensures that all declarations within this namespace have hidden
    // visibility, which optimizes codegen for uses of symbols defined in other
    // translation units in ways that can be necessary for correctness by avoiding
    // dynamic relocations. This does not affect the public C symbols which are
    // controlled independently via `LLVM_LIBC_FUNCTION_ATTR`.
    #define LIBC_NAMESPACE_DECL [[gnu::visibility("hidden")]] LIBC_NAMESPACE
#else
    // needs to be diagnosed and addressed.
    #define LIBC_NAMESPACE_DECL LIBC_NAMESPACE
#endif

#ifdef __clang__
    #define ALIGN(size) __attribute__((packed)) __attribute__((aligned(size)))
#else
    #define ALIGN(size)
#endif

#endif //CONFIG_H

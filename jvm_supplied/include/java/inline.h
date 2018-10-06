#ifndef JAVA_INLINE_H
#define JAVA_INLINE_H

///\file
///\brief Declares the JSER_INLINE macro, which forces inlining.

#if defined(_MSC_VER)
// Always inline for MS compiler.
# define JSER_INLINE inline __forceinline
#elif defined(__clang__) || defined(__GNUC__)
// Always inline for GNU and LLVM.
# define JSER_INLINE inline __attribute__((__visibility__("hidden"), __always_inline__))
#else
// Declare inline and cross our fingers.
// The code doesn't depend on inlined functions being erased entirely,
// so it's not a problem if the compiler does emit a non-inlined instance.
// It's just very inefficient, code wise.
# define JSER_INLINE inline
#endif

#endif /* JAVA_INLINE_H */

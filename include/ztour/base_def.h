#pragma once

// Architecture check
#if defined(_M_X64) || defined(__x86_64__)
#define ZT_IS_64 1
#define ZT_PTR_SIZE 8
#elif defined(_M_IX86) || defined(__i386__)
#define ZT_IS_64 0
#define ZT_PTR_SIZE 4
#error SCAM
#else
#error "Only x86 and x86_64 architectures are supported"
#endif

static_assert(sizeof(void*) == ZT_PTR_SIZE, "Bad pointer size");

// Compiler check
#if defined(_MSC_VER)
#define ZT_IS_MSVC 1
#else
#define ZT_IS_MSVC 0
#endif

// Operating system check
#if defined(_WIN32) || defined(_WIN64)
    #define ZT_IS_WINDOWS 1
    #define ZT_IS_UNIX 0
    #define ZT_IS_MACOS 0
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    #define ZT_IS_UNIX 1
    #define ZT_IS_WINDOWS 0
    #if defined(__APPLE__) && defined(__MACH__)
        #define ZT_IS_MACOS 1
        #define ZT_IS_LINUX 0
    #elif defined(__linux__)
        #define ZT_IS_MACOS 0
        #define ZT_IS_LINUX 1
    #else
        #error "Unsupported Unix-based operating system!"
    #endif
#else
    #error "Unsupported operating System! Only Windows and Unix-like systems are supported."
#endif

// Debug check
#if defined(NDEBUG)
#define ZT_IS_DEBUG 0
#else
#define ZT_IS_DEBUG 1
#endif

// Calling convention defs
#if ZT_IS_MSVC
#define ZT_CC_CDECL __cdecl
#define ZT_CC_STDCALL __stdcall
#define ZT_CC_FASTCALL __fastcall
#else
#define ZT_CC_CDECL __attribute__((cdecl))
#define ZT_CC_STDCALL __attribute__((stdcall))
#define ZT_CC_FASTCALL __attribute__((fastcall))
#endif

#if ZT_IS_MSVC
#define ZT_NO_INLINE __declspec(noinline)
#else
#define ZT_NO_INLINE __attribute__((noinline))
#endif

// When modifying text sections, there's an annoying special case where the section we are modifying is the same
// as the section we are executing, meaning we will crash because we set ourselves to read-write.
// Separating special code-modifying functions into a different code section fixes this.auto
#if defined(_MSC_VER)
#pragma section(".ztcore", read, execute)
#define ZT_ISOLATE_SECTION __declspec(allocate(".zttext")) ZT_NO_INLINE
#else
#define ZT_ISOLATE_SECTION __attribute__((section(".zttext"), aligned(4096))) ZT_NO_INLINE
#endif
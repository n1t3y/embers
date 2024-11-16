#include <cstdint>

// --- Some utils ---

#if defined(_WIN32)
#define EMBERS_DLL_IMPORT __declspec(dllimport)
#define EMBERS_DLL_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#define EMBERS_DLL_IMPORT __attribute__((visibility("default")))
#define EMBERS_DLL_EXPORT __attribute__((visibility("default")))
#else
#define EMBERS_DLL_IMPORT
#define EMBERS_DLL_EXPORT
#endif

#if !defined(EMBERS_DLL)
#define EMBERS_API
#elif defined(EMBERS_DLL_EXPORTS)
#define EMBERS_API EMBERS_DLL_EXPORT
#else
#define EMBERS_API EMBERS_DLL_IMPORT
#endif

#if defined(_MSC_VER)
#define EMBERS_ALWAYS_INLINE __forceinline
#else
#define EMBERS_ALWAYS_INLINE __attribute__((always_inline)) inline

#endif

#define _EMBERS__STRINGIFY_INTERNAL(x) #x
#define EMBERS_STRINGIFY(x)            _EMBERS__STRINGIFY_INTERNAL(x)

// is used by logger, use short names (when possible)
#if defined(__FILE_NAME__)
#define EMBERS_FILENAME __FILE_NAME__
#elif defined(__FILE__)
#define EMBERS_FILENAME __FILE__
#else
#define EMBERS_FILENAME "unknown_file"
#endif

#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define EMBERS_DEBUGBREAK() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define EMBERS_DEBUGBREAK() __debugbreak()
#endif
#endif
#if !defined(EMBERS_DEBUGBREAK)
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define EMBERS_DEBUGBREAK() __debugbreak()
#endif
#endif

// --- Types ---

/// Signed integer with width of 8bit
typedef int8_t i8;
#define i8_MAX INT8_MAX
#define i8_MIN INT8_MIN

/// Signed integer with width of 16bit
typedef int16_t i16;
#define i16_MAX INT16_MAX
#define i16_MIN INT16_MIN

/// Signed integer with width of 32bit
typedef int32_t i32;
#define i32_MAX INT32_MAX
#define i32_MIN INT32_MIN

/// Signed integer with width of 64bit
typedef int64_t i64;
#define i64_MAX INT64_MAX
#define i64_MIN INT64_MIN

/// Fastest signed integer with width of at least 8bit
typedef int_fast8_t i8f;
#define i8f_MAX INT_FAST8_MAX
#define i8f_MIN INT_FAST8_MIN

/// Fastest signed integer with width of at least 16bit
typedef int_fast16_t i16f;
#define i16f_MAX INT_FAST16_MAX
#define i16f_MIN INT_FAST16_MIN

/// Fastest signed integer with width of at least 32bit
typedef int_fast32_t i32f;
#define i32f_MAX INT_FAST32_MAX
#define i32f_MIN INT_FAST32_MIN

/// Fastest signed integer with width of at least 64bit
typedef int_fast64_t i64f;
#define i64f_MAX INT_FAST64_MAX
#define i64f_MIN INT_FAST64_MIN

/// Smallest signed integer with width of at least 8bit
typedef int_least8_t i8l;
#define i8l_MAX INT_LEAST8_MAX
#define i8l_MIN INT_LEAST8_MIN

/// Smallest signed integer with width of at least 16bit
typedef int_least16_t i16l;
#define i16l_MAX INT_LEAST16_MAX
#define i16l_MIN INT_LEAST16_MIN

/// Smallest signed integer with width of at least 32bit
typedef int_least32_t i32l;
#define i32l_MAX INT_LEAST32_MAX
#define i32l_MIN INT_LEAST32_MIN

/// Smallest signed integer with width of at least 64bit
typedef int_least64_t i64l;
#define i64l_MAX INT_LEAST64_MAX
#define i64l_MIN INT_LEAST64_MIN

/// Unsigned 8bit integer with width of at least 8bit
typedef uint8_t u8;
#define u8_MAX UINT8_MAX

/// Unsigned 16bit integer with width of at least 16bit
typedef uint16_t u16;
#define u16_MAX UINT16_MAX

/// Unsigned 32bit integer with width of at least 32bit
typedef uint32_t u32;
#define u32_MAX UINT32_MAX

/// Unsigned 64bit integer with width of at least 64bit
typedef uint64_t u64;
#define u64_MAX UINT64_MAX

/// Fastest unsigned integer with width of at least 8bit
typedef uint_fast8_t u8f;
#define u8f_MAX UINT_FAST8_MAX

/// Fastest unsigned integer with width of at least 16bit
typedef uint_fast16_t u16f;
#define u16f_MAX UINT_FAST16_MAX

/// Fastest unsigned integer with width of at least 32bit
typedef uint_fast32_t u32f;
#define u32f_MAX UINT_FAST32_MAX

/// Fastest unsigned integer with width of at least 64bit
typedef uint_fast64_t u64f;
#define u64f_MAX UINT_FAST64_MAX

/// Smallest unsigned integer with width of at least 8bit
typedef uint_least8_t u8l;
#define u8l_MAX UINT_LEAST8_MAX

/// Smallest unsigned integer with width of at least 16bit
typedef uint_least16_t u16l;
#define u16l_MAX UINT_LEAST16_MAX

/// Smallest unsigned integer with width of at least 32bit
typedef uint_least32_t u32l;
#define u32l_MAX UINT_LEAST32_MAX

/// Smallest unsigned integer with width of at least 64bit
typedef uint_least64_t u64l;
#define u64l_MAX UINT_LEAST64_MAX

/// Floating number with width of 32bits
typedef float f32;

/// Floating number with width of 64bits
typedef double f64;

/// Bool with width of 8bit
typedef bool b8;

static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

/// Bool with width of 32bit
typedef int32_t b32;

static_assert(sizeof(b8) == 1, "b8 must be 1 byte");
static_assert(sizeof(b32) == 4, "b32 must be 4 bytes");

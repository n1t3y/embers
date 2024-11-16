#pragma once

#include <fmt/format.h>

#include "defines.hpp"

#define EMBERS__LOG_LOCATION EMBERS_FILENAME ":" EMBERS_STRINGIFY(__LINE__)

#if !defined(EMBERS_CONFIG_DEBUG) && false
#define EMBERS_DEBUG(format, ...)
#define EMBERS_INFO(format, ...)
#define EMBERS_WARN(format, ...)
#define EMBERS_ASSERT(expr, ...)
#define EMBERS_ASSERT_WARN(expr, ...)
#else
#define EMBERS_DEBUG(format, ...)                                              \
  embers::logger::log(                                                         \
      embers::logger::Level::kDebug,                                           \
      EMBERS__LOG_LOCATION,                                                    \
      FMT_STRING(format),                                                      \
      __VA_ARGS__                                                              \
  )

#define EMBERS_INFO(format, ...)                                               \
  embers::logger::log(                                                         \
      embers::logger::Level::kInfo,                                            \
      EMBERS__LOG_LOCATION,                                                    \
      FMT_STRING(format),                                                      \
      __VA_ARGS__                                                              \
  )

#define EMBERS_WARN(format, ...)                                               \
  embers::logger::log(                                                         \
      embers::logger::Level::kWarn,                                            \
      EMBERS__LOG_LOCATION,                                                    \
      FMT_STRING(format),                                                      \
      __VA_ARGS__                                                              \
  )
// #define EMBERS_ASSERT(expr, ...)                                               \
//   do {                                                                         \
//     if (!(expr)) {                                                             \
//       embers::logger::log(                                                     \
//           embers::logger::Level::kError,                                       \
//           EMBERS__LOG_LOCATION,                      \
//           __VA_ARGS__                                                          \
//       );                                                                       \
//       fflush(stdout);                                                          \
//       fflush(stderr);                                                          \
//       EMBERS_DEBUGBREAK();                                                     \
//     }                                                                          \
//   } while (0)
#define EMBERS_ASSERT(expr, ...)

#define EMBERS_ASSERT_WARN(expr, ...)                                          \
  do {                                                                         \
    if (!(expr)) {                                                             \
      embers::logger::log(                                                     \
          embers::logger::Level::kError,                                       \
          EMBERS__LOG_LOCATION,                                                \
          __VA_ARGS__                                                          \
      );                                                                       \
    }                                                                          \
  } while (0)

#endif

#define EMBERS_ERROR(format, ...)                                              \
  embers::logger::log(                                                         \
      embers::logger::Level::kError,                                           \
      EMBERS__LOG_LOCATION,                                                    \
      FMT_STRING(format),                                                      \
      __VA_ARGS__                                                              \
  )

#define EMBERS_FATAL(format, ...)                                              \
  embers::logger::log(                                                         \
      embers::logger::Level::kFatal,                                           \
      EMBERS__LOG_LOCATION,                                                    \
      FMT_STRING(format),                                                      \
      __VA_ARGS__                                                              \
  )

namespace embers::logger {

enum class Level {
  kMin   = 0,
  kDebug = 0,
  kInfo  = 1,
  kWarn  = 2,
  kError = 3,
  kFatal = 4,
  kMax   = 4,
};

namespace internal {
void vlog(
    Level            level,
    const char*      system,
    fmt::string_view format,
    fmt::format_args args
);
}  // namespace internal

template <typename... T>
EMBERS_ALWAYS_INLINE void log(
    Level                    level,
    const char*              system,
    fmt::format_string<T...> format,
    T&&... args
);

EMBERS_ALWAYS_INLINE void vlog(
    Level            level,
    const char*      system,
    fmt::string_view format,
    fmt::format_args args
) {
  EMBERS_ASSERT(
      Level::kMax >= level && level >= Level::kMin,
      "Level is out of bounds"
  );
  static_assert(
      Level::kMax >= Level::kError && Level::kError >= Level::kMin,
      "embers::logger::Level::kError is out of bounds, "
      "the call will get into a recursion, that's bad"
  );
  return internal::vlog(level, system, format, args);
};

template <typename... T>
EMBERS_ALWAYS_INLINE void log(
    Level                    level,
    const char*              system,
    fmt::format_string<T...> format,
    T&&... args
) {
  return vlog(level, system, format, fmt::make_format_args(args...));
}

template <typename... T>
EMBERS_ALWAYS_INLINE void debug(
    const char* system, fmt::format_string<T...> format, T&&... args
) {
  return vlog(Level::kDebug, system, format, fmt::make_format_args(args...));
}

template <typename... T>
EMBERS_ALWAYS_INLINE void info(
    const char* system, fmt::format_string<T...> format, T&&... args
) {
  return vlog(Level::kInfo, system, format, fmt::make_format_args(args...));
}

template <typename... T>
EMBERS_ALWAYS_INLINE void warn(
    const char* system, fmt::format_string<T...> format, T&&... args
) {
  return vlog(Level::kWarn, system, format, fmt::make_format_args(args...));
}

template <typename... T>
EMBERS_ALWAYS_INLINE void error(
    const char* system, fmt::format_string<T...> format, T&&... args
) {
  return vlog(Level::kError, system, format, fmt::make_format_args(args...));
}

template <typename... T>
EMBERS_ALWAYS_INLINE void fatal(
    const char* system, fmt::format_string<T...> format, T&&... args
) {
  return vlog(Level::kFatal, system, format, fmt::make_format_args(args...));
}

}  // namespace embers::logger

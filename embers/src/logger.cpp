#include <embers/defines.hpp>
#include <embers/logger.hpp>
#include <iterator>

#include "containers/allocator.hpp"
#include "containers/debug_allocator.hpp"

#ifdef EMBERS_CONFIG_DEBUG
template <typename T>
using Allocator = embers::containers::with<
    embers::containers::DefaultAllocator,
    embers::containers::DebugAllocatorTags::kLogger>::DebugAllocator<T>;

#else
template <typename T>
using Allocator = embers::containers::DefaultAllocator<T>;
#endif

using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

using MemoryBuffer =
    fmt::basic_memory_buffer<char, fmt::inline_buffer_size, Allocator<char>>;

static Allocator<char> allocator = {};

String vformat(
    Allocator<char> alloc, fmt::string_view format_str, fmt::format_args args
) {
  auto buf = MemoryBuffer(alloc);
  fmt::vformat_to(std::back_inserter(buf), format_str, args);
  return String(buf.data(), buf.size(), alloc);
}

template <typename... Args>
inline String format(
    Allocator<char> alloc, fmt::string_view format_str, const Args &...args
) {
  return vformat(alloc, format_str, fmt::make_format_args(args...));
}

namespace embers::logger {

constexpr const char *LOG_FILE_NAME = "log.txt";
constexpr const i32   LOG_LEVELS    = (int)Level::kMax - (int)Level::kMin + 1;

static const struct {
  fmt::string_view console;
  fmt::string_view file;
} FORMATS[LOG_LEVELS] = {
    {"\033[30;106m[Debug]\033[0m\033[36m @ {:{}} > {}\033[0m\n",
     "[Debug] @ {:{}} > {}\n"},
    {"\033[30;102m[Info]\033[0m \033[32m @ {:{}} > {}\033[0m\n",
     "[Info]  @ {:{}} > {}\n"},
    {"\033[30;103m[Warn]\033[0m \033[33m @ {:{}} > {}\033[0m\n",
     "[Warn]  @ {:{}} > {}\n"},
    {"\033[30;101m[Error]\033[0m\033[31m @ {:{}} > {}\033[0m\n",
     "[Error] @ {:{}} > {}\n"},
    {"\033[97;101m[╯°□°╯]\033[0m\033[31m @ {:{}} > {}\033[0m\n",
     "[Fatal] @ {:{}} > {}\n"}
};

static FILE *open_log_file();

void internal::vlog(
    Level            level,
    const char      *system,
    fmt::string_view format,
    fmt::format_args args
);

}  // namespace embers::logger

// implementation

namespace embers::logger {

static FILE *open_log_file() {
  const char *filename = LOG_FILE_NAME;
  FILE       *log_file;
  errno_t     err = fopen_s(&log_file, filename, "w");
  if (err == 0) {
    return log_file;
  }
  // unable to open file: log the error into console
  const u32 error_string_maxlen               = 128;
  char      error_string[error_string_maxlen] = {};
  errno_t   errno_string = strerror_s(error_string, error_string_maxlen, err);
  String    formatted_error;

  fmt::print(
      stderr,
      fmt::runtime(FORMATS[(int)Level::kError].console),
      EMBERS_FILENAME ":" EMBERS_STRINGIFY(__LINE__),
      errno_string == 0 ? formatted_error = fmt::format(
                              "Unable to open log file {}; Error: {}",
                              filename,
                              error_string
                          )
                        : formatted_error = fmt::format(
                              "Unable to open log file {}; Errno: {}",
                              filename,
                              err
                          )
  );
  return nullptr;
}

void internal::vlog(
    Level            level,
    const char      *system,
    fmt::string_view format,
    fmt::format_args args
) {
  static size_t system_width = 8;
  const String  message      = vformat(allocator, format, args);
  const auto    formats      = FORMATS[(int)level];

  system_width = std::max(strlen(system), system_width);

  // Write to stdout/stderr
  fmt::print(
      level >= Level::kError ? stderr : stdout,
      fmt::runtime(formats.console),
      system,
      system_width,
      message
  );

  // Write to file (if possible)
  static FILE *log_file = open_log_file();
  if (log_file != nullptr) {
    fmt::print(
        log_file,
        fmt::runtime(formats.file),
        system,
        system_width,
        message
    );
  }
  return;
}

}  // namespace embers::logger
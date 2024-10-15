#include <embers/defines/types.hpp>
#include <embers/defines/utils.hpp>
#include <embers/logger.hpp>

const i32 LOG_LEVELS = 5;

static const struct {
  fmt::string_view console;
  fmt::string_view file;
} FORMATS[LOG_LEVELS] = {
    {"\033[30;106m[Debug]\033[0m\033[36m @ {:8} > {}\033[0m\n", "[Debug] @ {:8} > {}\n"},
    {"\033[30;102m[Info]\033[0m \033[32m @ {:8} > {}\033[0m\n", "[Info]  @ {:8} > {}\n"},
    {"\033[30;103m[Warn]\033[0m \033[33m @ {:8} > {}\033[0m\n", "[Warn]  @ {:8} > {}\n"},
    {"\033[30;101m[Error]\033[0m\033[31m @ {:8} > {}\033[0m\n", "[Error] @ {:8} > {}\n"},
    {"\033[97;101m[╯°□°╯]\033[0m\033[31m @ {:8} > {}\033[0m\n", "[Fatal] @ {:8} > {}\n"}
};

static FILE *open_log_file() {
  const char *filename = "log.txt";
  FILE       *log_file;
  errno_t     err = fopen_s(&log_file, filename, "w");
  if (err == 0) {
    return log_file;
  }
  // unable to open file: log the error into console
  const u32   error_string_maxlen               = 128;
  char        error_string[error_string_maxlen] = {};
  errno_t     errno_string                      = strerror_s(error_string, error_string_maxlen, err);
  std::string formatted_error;

  if (errno_string == 0) {
    formatted_error = fmt::format("Unable to open log file {}; Error: {}", filename, error_string);
  } else {
    formatted_error = fmt::format("Unable to open log file {}; Errno: {}", filename, err);
  }
  fmt::print(
      stdout, fmt::runtime(FORMATS[(int)embers::logger::Level::kError].console),
      EMBERS_FILENAME ":" EMBERS_STRINGIFY(__LINE__), formatted_error
  );
  return nullptr;
}

void embers::logger::internal::vlog(Level level, const char *system, fmt::string_view format, fmt::format_args args) {
  const std::string message = fmt::vformat(format, args);

  const auto formats = FORMATS[(int)level];

  // Write to stdout/stderr
  fmt::print(level >= Level::kError ? stderr : stdout, fmt::runtime(formats.console), system, message);

  // Write to file (if possible)
  static FILE *log_file = open_log_file();
  if (log_file != nullptr) {
    fmt::print(log_file, fmt::runtime(formats.file), system, message);
  }
}

static_assert(
    (int)embers::logger::Level::kMax - (int)embers::logger::Level::kMin == LOG_LEVELS - 1,
    "Log level strings must not be out of bounds"
);
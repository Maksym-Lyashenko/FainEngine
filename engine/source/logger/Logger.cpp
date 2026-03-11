#include "logger/Logger.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// windows.h defines ERROR as a macro; undefine to avoid conflicts with LogLevel::ERROR.
#undef ERROR
#endif

namespace eng
{

// Static member definitions.
std::ofstream Logger::logFile;
std::atomic<bool> Logger::colorsEnabled{true};
std::atomic<int> Logger::minLevel{static_cast<int>(LogLevel::Info)};

namespace
{
/// Global mutex guarding access to the logger state and outputs.
/// Ensures that log lines from different threads do not interleave.
std::mutex g_logMutex;

/**
 * @brief Build a human-readable timestamp string with millisecond precision.
 *
 * Format: "YYYY-MM-DD HH:MM:SS.mmm"
 *
 * This function is used for each log line, so it should remain reasonably
 * efficient, but logging is typically not in a performance-critical path.
 */
inline std::string makeTimestamp()
{
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t t = system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  // Thread-safe localtime on Windows.
  localtime_s(&tm, &t);
#else
  // Thread-safe localtime on POSIX.
  localtime_r(&t, &tm);
#endif

  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3)
      << ms.count();
  return oss.str();
}

/**
 * @brief Build a timestamp suffix for log file names.
 *
 * Format: "YYYY-MM-DD_HH-MM-SS"
 *
 * This is used to create unique log file names per run, e.g.:
 * "engine_2025-11-17_01-20-39.log".
 */
inline std::string makeFileSuffix()
{
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t t = system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
  return oss.str();
}

}  // namespace

void Logger::initConsoleVT() noexcept
{
#ifdef _WIN32
  // On Windows, enable virtual terminal processing so that ANSI escape
  // sequences are interpreted correctly by the console.
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
  {
    return;
  }

  DWORD mode = 0;
  if (!GetConsoleMode(hOut, &mode))
  {
    return;
  }

  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, mode);
#endif
}

void Logger::init(const std::string& baseFilename) noexcept
{
  std::lock_guard<std::mutex> lock(g_logMutex);

  // Try to enable ANSI colors on platforms that require explicit setup.
  initConsoleVT();

  // Ensure directory for the base file path exists (if any).
  std::filesystem::path basePath(baseFilename);
  if (basePath.has_parent_path())
  {
    std::error_code ec;
    std::filesystem::create_directories(basePath.parent_path(), ec);
    // Errors are intentionally ignored here; if directory creation fails,
    // the subsequent file open may fail and will be reported.
  }

  // Compose final path: e.g. logs/engine_YYYY-MM-DD_HH-MM-SS.log
  const std::string suff = makeFileSuffix();
  std::filesystem::path finalPath = basePath.parent_path() / (basePath.stem().string() + "_" +
                                                              suff + basePath.extension().string());

  // Reset any previous file stream state and open the new log file.
  logFile.close();
  logFile.clear();
  logFile.open(finalPath, std::ios::out | std::ios::trunc);

  if (!logFile.is_open())
  {
    // Fall back to console error if file cannot be opened.
    std::cerr << "Failed to open log file: " << finalPath << std::endl;
  }
  else
  {
    // Notify user where logs are being written.
    std::cout << "Logging to file: " << finalPath << std::endl;
  }
}

void Logger::shutdown() noexcept
{
  std::lock_guard<std::mutex> lock(g_logMutex);
  if (logFile.is_open())
  {
    logFile.close();
  }
}

void Logger::setLevel(LogLevel level) noexcept
{
  // Relaxed ordering is sufficient since we only need eventual consistency
  // for log filtering across threads.
  minLevel.store(static_cast<int>(level), std::memory_order_relaxed);
}

void Logger::enableColors(bool enabled) noexcept
{
  colorsEnabled.store(enabled, std::memory_order_relaxed);
}

void Logger::log(LogLevel level, const std::string& message) noexcept
{
  // Fast path: discard messages below the current minimal log level
  // before doing any expensive work (timestamping, formatting, locking).
  if (static_cast<int>(level) < minLevel.load(std::memory_order_relaxed))
  {
    return;  // filtered out
  }

  const std::string ts = makeTimestamp();
  const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

  const char* lvlStr = levelToString(level);
  const bool useColors = colorsEnabled.load(std::memory_order_relaxed);
  const char* col = useColors ? levelToColor(level) : "";
  const char* reset = useColors ? "\033[0m" : "";

  std::lock_guard<std::mutex> lock(g_logMutex);

  // Console output with optional color prefix and reset suffix.
  std::cout << col << '[' << ts << "][" << lvlStr << "][t:" << tid << "] " << message << reset
            << std::endl;

  // File output (no colors). If the file is not open, this silently does nothing.
  if (logFile.is_open())
  {
    logFile << '[' << ts << "][" << lvlStr << "][t:" << tid << "] " << message << std::endl;
    // Optionally, we could flush here to reduce log loss on crashes,
    // at the cost of performance:
    // logFile.flush();
  }
}

const char* Logger::levelToString(LogLevel level) noexcept
{
  switch (level)
  {
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warning:
      return "WARN";
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Debug:
      return "DEBUG";
  }
  return "UNKNOWN";
}

const char* Logger::levelToColor(LogLevel level) noexcept
{
  // ANSI color codes for different severity levels.
  switch (level)
  {
    case LogLevel::Info:
      return "\033[36m";  // cyan
    case LogLevel::Warning:
      return "\033[33m";  // yellow
    case LogLevel::Error:
      return "\033[31m";  // red
    case LogLevel::Debug:
      return "\033[35m";  // magenta
  }
  // Default: reset color.
  return "\033[0m";
}

}  // namespace eng

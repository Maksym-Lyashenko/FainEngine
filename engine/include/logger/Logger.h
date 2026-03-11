#pragma once

#include <string>
#include <fstream>
#include <atomic>

namespace eng
{

/**
 * @brief Log severity levels used by the Logger.
 *
 * The numeric order of these enum values is important, since it is used
 * for filtering messages based on the current minimal log level.
 */
enum class LogLevel
{
  Info,     ///< Informational messages about normal engine operation.
  Warning,  ///< Recoverable issues or suspicious situations.
  Error,    ///< Serious problems that typically indicate failures.
  Debug     ///< Verbose diagnostic output, usually disabled in release builds.
};

/**
 * @brief Simple thread-safe logger to console and file.
 *
 * This logger provides:
 * - A global, process-wide logging sink.
 * - Thread-safe writes via an internal mutex (messages from different threads
 *   will not interleave on a single line).
 * - Automatic timestamping and thread ID tagging for each message.
 * - Optional ANSI color output in the console for improved readability.
 * - A timestamped log file name based on a user-provided base filename.
 *
 * Typical usage pattern:
 * @code
 * Logger::init("logs/engine.log");    // creates logs/engine_YYYY-MM-DD_HH-MM-SS.log
 * Logger::setLevel(LogLevel::INFO); // optional: filter out lower severities
 * LOG_INFO("Hello");
 * Logger::shutdown();                 // close log file when shutting down
 * @endcode
 */
class Logger
{
 public:
  /**
   * @brief Initialize the logger and open a timestamped log file.
   *
   * This function:
   * - Ensures that the parent directory of @p baseFilename exists.
   * - Derives a final path in the form:
   *   "<dir>/<stem>_YYYY-MM-DD_HH-MM-SS<ext>"
   *   e.g., "logs/engine_2025-11-17_01-20-39.log".
   * - Opens the file for writing (truncating any existing file with same name).
   * - Optionally prints the final log file path to std::cout.
   * - On Windows, attempts to enable ANSI color escape sequences for the console.
   *
   * If the file cannot be opened, an error message is printed to std::cerr.
   *
   * @param baseFilename Base path for the log file (e.g. "logs/engine.log").
   */
  static void init(const std::string& filename = "engine.log") noexcept;

  /**
   * @brief Shut down the logger and close the log file.
   *
   * It is safe to call this multiple times; subsequent calls after the file
   * has been closed are no-ops. After shutdown(), log() will still print
   * to the console, but will no longer write to the file.
   */
  static void shutdown() noexcept;

  /**
   * @brief Log a message at a given severity level.
   *
   * The message is written both to:
   * - Console (std::cout) with optional ANSI color codes.
   * - Log file (if open), without color codes.
   *
   * Each log line has the following format:
   *   [YYYY-MM-DD HH:MM:SS.mmm][LEVEL][t:<thread_id>] message
   *
   * Messages with severity below the currently configured minimal log level
   * (see setLevel()) are ignored.
   *
   * @param level   Severity level for this message.
   * @param message Text to be logged (single line; caller should insert newlines if needed).
   */
  static void log(LogLevel level, const std::string& message) noexcept;

  /**
   * @brief Set the minimal severity level that will be printed/stored.
   *
   * For example, if minLevel is set to WARNING, then INFO and DEBUG messages
   * will be filtered out and not written to either console or file.
   *
   * Default value is LogLevel::INFO.
   *
   * @param level New minimal log level.
   */
  static void setLevel(LogLevel level) noexcept;

  /**
   * @brief Enable or disable ANSI color output in the console.
   *
   * When enabled, log lines are prefixed with color escape codes depending
   * on their severity level (INFO=WARN=yellow, ERROR=red, etc.). The log file
   * is always written without color codes.
   *
   * On Windows, init() attempts to enable virtual terminal processing so
   * that ANSI codes are interpreted correctly.
   *
   * @param enabled True to enable colored console output, false to disable.
   */
  static void enableColors(bool enabled) noexcept;

 private:
  /// Global log file stream used by the Logger.
  static std::ofstream logFile;

  /// Flag that controls whether ANSI color codes are emitted to the console.
  static std::atomic<bool> colorsEnabled;

  /// Minimal log level to be emitted. Messages below this level are discarded.
  static std::atomic<int> minLevel;

  /// Convert a LogLevel to a short string (e.g., "INFO", "ERROR").
  static const char* levelToString(LogLevel level) noexcept;

  /// Return ANSI color escape sequence for the given level (console only).
  static const char* levelToColor(LogLevel level) noexcept;

  /// Platform-specific helper: on Windows, enable ANSI escape sequence support.
  static void initConsoleVT() noexcept;  // Windows: enable ANSI colors
};

}  // namespace eng

// Convenience macros for shorter logging calls. These simply forward
// to Logger::log with the appropriate log level.
#define LOG_INFO(msg) ::eng::Logger::log(::eng::LogLevel::Info, (msg))
#define LOG_WARN(msg) ::eng::Logger::log(::eng::LogLevel::Warning, (msg))
#define LOG_ERROR(msg) ::eng::Logger::log(::eng::LogLevel::Error, (msg))
#define LOG_DEBUG(msg) ::eng::Logger::log(::eng::LogLevel::Debug, (msg))

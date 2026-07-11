#pragma once

#ifndef __CLOGGER_ESP32_H__
#define __CLOGGER_ESP32_H__

#include <inttypes.h>
#include <stdarg.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <vector>

#define CLOGGER_SEVERITY_VERBOSE (1)
#define CLOGGER_SEVERITY_DEBUG (2)
#define CLOGGER_SEVERITY_INFO (3)
#define CLOGGER_SEVERITY_WARN (4)
#define CLOGGER_SEVERITY_ERROR (5)
#define CLOGGER_SEVERITY_FATAL (6)
#define CLOGGER_SEVERITY_NONE (7)

#ifndef CLOGGER_SEVERITY
#define CLOGGER_SEVERITY CLOGGER_SEVERITY_INFO
#endif

class Clogger {
 public:
  static constexpr char SeverityToChar(const int severity) {
    switch (severity) {
      case CLOGGER_SEVERITY_VERBOSE:
        return 'V';
      case CLOGGER_SEVERITY_DEBUG:
        return 'D';
      case CLOGGER_SEVERITY_INFO:
        return 'I';
      case CLOGGER_SEVERITY_WARN:
        return 'W';
      case CLOGGER_SEVERITY_ERROR:
        return 'E';
      case CLOGGER_SEVERITY_FATAL:
        return 'F';
    }
    return 'X';
  }

  template <typename T, size_t size>
  static constexpr size_t FileNameOffset(const T (&file_path)[size], size_t i = size) {
    return (i == 0) ? 0 : (file_path[i - 1] == '/' || file_path[i - 1] == '\\') ? i : FileNameOffset(file_path, i - 1);
  }

  static void Trace(const char *file_name, const uint32_t line_num, const char *function) {
    auto now = std::chrono::system_clock::now();
    auto time_sec = std::chrono::system_clock::to_time_t(now);
    auto time_since_epoch = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch % std::chrono::seconds(1)).count();
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_sec);
#else
    localtime_r(&time_sec, &tm);
#endif
    printf("%02d:%02d:%02d.%03lld %s:%" PRIu32 " %s]\n", tm.tm_hour, tm.tm_min, tm.tm_sec, ms, file_name, line_num, function);
  }

  static void Log(const int32_t serverity, const char *file_name, const uint32_t line_num, const char *function, const char *fmt, ...) {
    auto now = std::chrono::system_clock::now();
    auto time_sec = std::chrono::system_clock::to_time_t(now);
    auto time_since_epoch = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch % std::chrono::seconds(1)).count();
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_sec);
#else
    localtime_r(&time_sec, &tm);
#endif

    const auto header_fmt = "%02d:%02d:%02d.%03lld %c %s:%" PRIu32 " %s] ";
    const auto header_length =
        snprintf(nullptr, 0, header_fmt, tm.tm_hour, tm.tm_min, tm.tm_sec, ms, SeverityToChar(serverity), file_name, line_num, function);

    va_list args;
    va_start(args, fmt);
    const auto message_length = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    const size_t total_size = header_length + message_length + 1;
    std::vector<char> buffer(total_size);

    snprintf(buffer.data(), total_size, header_fmt, tm.tm_hour, tm.tm_min, tm.tm_sec, ms, SeverityToChar(serverity), file_name, line_num, function);

    va_start(args, fmt);
    vsnprintf(buffer.data() + header_length, message_length + 1, fmt, args);
    va_end(args);
    fwrite(buffer.data(), 1, buffer.size(), stdout);
  }
};

#define CLOG(fmt, ...) CLOGI(fmt, ##__VA_ARGS__)

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_VERBOSE
#define CLOGV(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_VERBOSE, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGV(fmt, ...)
#endif

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_DEBUG
#define CLOGD(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_DEBUG, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGD(fmt, ...)
#endif

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_INFO
#define CLOGI(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_INFO, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGI(fmt, ...)
#endif

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_WARN
#define CLOGW(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_WARN, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGW(fmt, ...)
#endif

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_ERROR
#define CLOGE(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_ERROR, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGE(fmt, ...)
#endif

#if CLOGGER_SEVERITY <= CLOGGER_SEVERITY_FATAL
#define CLOGF(fmt, ...) \
  Clogger::Log(CLOGGER_SEVERITY_FATAL, __FILE__ + Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, fmt "\n", ##__VA_ARGS__)
#else
#define CLOGF(fmt, ...)
#endif

#endif
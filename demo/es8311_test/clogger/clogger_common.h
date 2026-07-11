#pragma once

#ifndef __CLOGGER_COMMON_H__
#define __CLOGGER_COMMON_H__

#include <Arduino.h>
#include <WString.h>
#include <stdarg.h>
#include <stdio.h>

struct Clogger {
  template <typename T, size_t size>
  static constexpr size_t FileNameOffset(const T (&file_path)[size], size_t i = size - 1) {
    static_assert(size > 1, "");
    return file_path[i] == '/' || file_path[i] == '\\' ? i + 1 : (i == 0 ? 0 : FileNameOffset(file_path, i - 1));
  }

  static void Log(const __FlashStringHelper *file_name,
                  const size_t file_name_begin,
                  const size_t line_num,
                  const char *function,
                  const __FlashStringHelper *fmt,
                  ...) {
    const auto now = millis();
    const uint_fast16_t millis = now % 1000;
    const uint_fast8_t seconds = now / 1000 % 60;
    const uint_fast8_t minutes = now / 60000 % 60;
    const uint_fast8_t hours = now / 3600000 % 100;

    char time_str[14];
    snprintf_P(time_str,
               sizeof(time_str),
               reinterpret_cast<const char *>(F("%02" PRIuFAST8 ":%02" PRIuFAST8 ":%02" PRIuFAST8 ".%03" PRIuFAST16 " ")),
               hours,
               minutes,
               seconds,
               millis);
    Serial.print(time_str);
    Serial.print(reinterpret_cast<const __FlashStringHelper *>(reinterpret_cast<PGM_P>(file_name) + file_name_begin));
    Serial.print(':');
    Serial.print(line_num);
    Serial.print(' ');
    Serial.print(function);
    Serial.print(F("] "));

    if (nullptr == fmt) {
      return;
    }

    va_list args;
    va_start(args, fmt);
    const auto size = vsnprintf_P(nullptr, 0, reinterpret_cast<const char *>(fmt), args);
    va_end(args);

    char *buffer = new char[size + 1];

    va_start(args, fmt);
    vsnprintf_P(buffer, size + 1, reinterpret_cast<const char *>(fmt), args);
    va_end(args);
    Serial.println(buffer);
    delete[] buffer;
  }
};  // namespace clog

#define CLOG(fmt, ...) Clogger::Log(F(__FILE__), Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, F("" fmt), ##__VA_ARGS__)

#define CLOG_TRACE() Clogger::Log(F(__FILE__), Clogger::FileNameOffset(__FILE__), __LINE__, __FUNCTION__, nullptr)

#endif
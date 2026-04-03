#pragma once

#include "log_severity.h"
#include "span.h"

#include "async/async.h"

#include <chrono>
#include <string>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

void Log(
  const Span&,
  std::chrono::steady_clock::time_point,
  LogSeverity,
  const std::string&);

void Log(
  const Span&,
  std::chrono::steady_clock::time_point,
  LogSeverity,
  std::string&&);

// -----------------------------------------------------------------------------

} // server

// -----------------------------------------------------------------------------

#define LOG(severity, message)        \
  Log(                                \
    server::GetActiveSpan(),          \
    std::chrono::steady_clock::now(), \
    severity,                         \
    message)
#define LOG_SPAN(span, severity, message) \
  Log(                                    \
    span,                                 \
    std::chrono::steady_clock::now(),     \
    severity,                             \
    message)
#define LOG_TIME(time, severity, message) \
  Log(                                    \
    server::GetActiveSpan(),              \
    time,                                 \
    severity,                             \
    message)
#define LOG_SPAN_TIME(span, time, severity, message) \
  Log(                                               \
    span,                                            \
    time,                                            \
    severity,                                        \
    message)
#define LOG_DEBUG(message) LOG(server::LogSeverity::DEBUG, message)
#define LOG_INFO (message) LOG(server::LogSeverity::INFO,  message)
#define LOG_WARN (message) LOG(server::LogSeverity::WARN,  message)
#define LOG_ERROR(message) LOG(server::LogSeverity::ERROR, message)
#define LOG_FATAL(message) LOG(server::LogSeverity::FATAL, message)

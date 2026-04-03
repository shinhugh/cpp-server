#include "log.h"

#include "log_manager.h"
#include "log_severity.h"
#include "span.h"

#include <chrono>
#include <optional>
#include <string>
#include <utility>

// -----------------------------------------------------------------------------

void server::Log(
  const Span& span,
  std::chrono::steady_clock::time_point time,
  LogSeverity severity,
  const std::string& message)
{
  g_logManager->Log(span, time, severity, message);
}

// -----------------------------------------------------------------------------

void server::Log(
  const Span& span,
  std::chrono::steady_clock::time_point time,
  LogSeverity severity,
  std::string&& message)
{
  g_logManager->Log(span, time, severity, std::move(message));
}

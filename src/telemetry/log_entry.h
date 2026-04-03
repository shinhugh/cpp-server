#pragma once

#include "log_severity.h"
#include "span.h"

#include <chrono>
#include <string>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

struct LogEntry
{
  Span m_span;
  std::chrono::steady_clock::time_point m_time;
  LogSeverity m_severity;
  std::string m_message;
};

// -----------------------------------------------------------------------------

} // server

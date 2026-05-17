#pragma once

#include <chrono>
#include <optional>
#include <string>

// -----------------------------------------------------------------------------

namespace telemetry
{

// -----------------------------------------------------------------------------

struct Span
{
  std::string m_traceId;
  std::string m_spanId;
  std::optional<std::string> m_parentSpanId;
  std::chrono::steady_clock::time_point m_startTime;
  std::chrono::steady_clock::time_point m_finishTime;
};

// -----------------------------------------------------------------------------

}  // namespace telemetry

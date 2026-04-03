#include "stdout_log_sink.h"

#include "log_entry.h"
#include "log_severity.h"
#include "span.h"

#include <chrono>
#include <iostream>
#include <optional>
#include <string>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

static std::ostream& operator<<(std::ostream&, LogSeverity);

// -----------------------------------------------------------------------------

} // server

// -----------------------------------------------------------------------------

std::optional<server::StdoutLogSink> server::g_stdoutLogSink;

// -----------------------------------------------------------------------------

void server::StdoutLogSink::Commit(const LogEntry& entry)
{
  std::cout
    << '[' << entry.m_time.time_since_epoch().count() << ']'
    << "[trace_id:" << entry.m_span.m_traceId
    << ",span_id:" << entry.m_span.m_spanId;
  if (entry.m_span.m_parentSpanId)
  {
    std::cout << ",parent_span_id:" << *entry.m_span.m_parentSpanId;
  }
  std::cout
    << ']'
    << '<' << entry.m_severity << '>'
    << entry.m_message
    << std::endl;
}

// -----------------------------------------------------------------------------

static std::ostream& server::operator<<(std::ostream& os, LogSeverity s)
{
  switch (s)
  {
  case LogSeverity::DEBUG:
    return os << "DEBUG";
  case LogSeverity::INFO:
    return os << "INFO";
  case LogSeverity::WARN:
    return os << "WARN";
  case LogSeverity::ERROR:
    return os << "ERROR";
  case LogSeverity::FATAL:
    return os << "FATAL";
  default:
    return os << "UNKNOWN";
  }
}

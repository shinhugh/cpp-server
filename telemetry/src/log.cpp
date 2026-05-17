#include "log.h"

#include "log_severity.h"
#include "log_value.h"
#include "span.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

std::vector<telemetry::impl::LogEntry> telemetry::impl::g_logEntries;
std::mutex telemetry::impl::g_logEntriesMutex;
static std::vector<std::function<void(
    const telemetry::Span &, std::chrono::system_clock::time_point,
    telemetry::LogSeverity, const std::string &,
    const std::vector<std::pair<std::string, telemetry::LogValue>> &)>>
    s_logHandlers;
static std::mutex s_logHandlersMutex;

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture()
{
  return LogValue::CreateVoid();
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    bool v)
{
  return LogValue::CreateBoolean(v);
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    long long v)
{
  return LogValue::CreateInteger(v);
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    double v)
{
  return LogValue::CreateFloat(v);
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    std::string &&v)
{
  return LogValue::CreateString(std::move(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    std::vector<LogValue> &&v)
{
  return LogValue::CreateArray(std::move(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    std::unordered_map<std::string, LogValue> &&v)
{
  return LogValue::CreateObject(std::move(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    int v)
{
  return Capture(static_cast<long long>(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    unsigned int v)
{
  return Capture(static_cast<long long>(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    long v)
{
  return Capture(static_cast<long long>(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    unsigned long v)
{
  return Capture(static_cast<long long>(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    float v)
{
  return Capture(static_cast<double>(v));
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    const std::string &v)
{
  return Capture(std::string{v});
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    const char *v)
{
  return Capture(std::string{v});
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::Capture(
    const void *v)
{
  std::stringstream s;
  s << v;
  return Capture(s.str());
}

// -----------------------------------------------------------------------------

void telemetry::FlushLogs()
{
  std::vector<impl::LogEntry> entries;
  {
    std::lock_guard lock{impl::g_logEntriesMutex};
    entries.swap(impl::g_logEntries);
  }

  {
    std::lock_guard lock{s_logHandlersMutex};
    for (impl::LogEntry &entry : entries)
    {
      for (
          std::function<void(
              const Span &, std::chrono::system_clock::time_point, LogSeverity,
              const std::string &,
              const std::vector<std::pair<std::string, LogValue>> &)>
              &logHandler : s_logHandlers)
      {
        logHandler(
            entry.m_span, entry.m_time, entry.m_severity, entry.m_event,
            entry.m_fields);
      }
    }
  }
}

// -----------------------------------------------------------------------------

void telemetry::RegisterLogHandler(
    std::function<void(
        const Span &, std::chrono::system_clock::time_point, LogSeverity,
        const std::string &,
        const std::vector<std::pair<std::string, LogValue>> &)> &&logHandler)
{
  std::lock_guard lock{s_logHandlersMutex};
  s_logHandlers.push_back(std::move(logHandler));
}

// -----------------------------------------------------------------------------

void telemetry::impl::AddFieldsToList(
    std::vector<std::pair<std::string, LogValue>> &)
{
}

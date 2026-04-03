#pragma once

#include "log_entry.h"
#include "log_severity.h"
#include "log_sink.h"
#include "span.h"

#include "subsystem/manager.h"

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

class LogManager : public Manager
{
public:
  virtual void Process() override;
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
  void FlushLogQueue();
  void RegisterLogSink(LogSink&);
  void UnregisterLogSink(LogSink&);

private:
  std::vector<server::LogEntry> m_entries;
  std::mutex m_entriesMutex;
  std::unordered_set<server::LogSink*> m_logSinks;
  std::mutex m_logSinksMutex;
};

// -----------------------------------------------------------------------------

extern std::optional<LogManager> g_logManager;

// -----------------------------------------------------------------------------

} // server

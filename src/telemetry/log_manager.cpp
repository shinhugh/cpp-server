#include "log_manager.h"

#include "log_entry.h"
#include "log_severity.h"
#include "log_sink.h"
#include "span.h"

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

std::optional<server::LogManager> server::g_logManager;

// -----------------------------------------------------------------------------

void server::LogManager::Process()
{
  FlushLogQueue();
}

// -----------------------------------------------------------------------------

void server::LogManager::Log(
  const Span& span,
  std::chrono::steady_clock::time_point time,
  LogSeverity severity,
  const std::string& message)
{
  LogEntry entry{ span, time, severity, message };
  std::lock_guard<std::mutex> lock{ m_entriesMutex };
  m_entries.push_back(std::move(entry));
}

// -----------------------------------------------------------------------------

void server::LogManager::Log(
  const Span& span,
  std::chrono::steady_clock::time_point time,
  LogSeverity severity,
  std::string&& message)
{
  LogEntry entry{ span, time, severity, std::move(message) };
  std::lock_guard<std::mutex> lock{ m_entriesMutex };
  m_entries.push_back(std::move(entry));
}

// -----------------------------------------------------------------------------

void server::LogManager::FlushLogQueue()
{
  std::vector<LogEntry> entries;
  {
    std::lock_guard<std::mutex> lock{ m_entriesMutex };
    entries.swap(m_entries);
  }
  for (const LogEntry& entry : entries)
  {
    std::lock_guard lock{ m_logSinksMutex };
    for (LogSink* logSink : m_logSinks)
    {
      logSink->Commit(entry);
    }
  }
}

// -----------------------------------------------------------------------------

void server::LogManager::RegisterLogSink(LogSink& logSink)
{
  std::lock_guard lock{ m_logSinksMutex };
  m_logSinks.insert(&logSink);
}

// -----------------------------------------------------------------------------

void server::LogManager::UnregisterLogSink(LogSink& logSink)
{
  std::lock_guard lock{ m_logSinksMutex };
  m_logSinks.erase(&logSink);
}

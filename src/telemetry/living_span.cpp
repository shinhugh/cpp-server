#include "living_span.h"

#include "log_severity.h"
#include "log.h"
#include "span.h"

#if defined(PLATFORM_POSIX)

#include <limits.h>
#include <unistd.h>

#elif defined(PLATFORM_WINDOWS)

#include <Windows.h>

#endif

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

static std::string GenerateProcessName();
static std::string GenerateSpanId(std::chrono::steady_clock::time_point time);
static std::string GenerateSpanId(
  std::chrono::steady_clock::time_point time,
  const std::vector<std::string>& tags);

// -----------------------------------------------------------------------------

static const std::string s_processName = GenerateProcessName();
static std::atomic<uint16_t> s_salt = 0;

// -----------------------------------------------------------------------------

server::LivingSpan server::LivingSpan::Create()
{
  std::chrono::steady_clock::time_point startTime
    = std::chrono::steady_clock::now();

  LivingSpan livingSpan;

  std::string spanId = GenerateSpanId(startTime);

  livingSpan.m_traceId = spanId;
  livingSpan.m_spanId = std::move(spanId);
  livingSpan.m_alive = true;
  livingSpan.m_startTime = startTime;

  return livingSpan;
}

// -----------------------------------------------------------------------------

server::LivingSpan server::LivingSpan::Create(
  const std::vector<std::string>& tags)
{
  std::chrono::steady_clock::time_point startTime
    = std::chrono::steady_clock::now();

  LivingSpan livingSpan;

  std::string spanId = GenerateSpanId(startTime, tags);

  livingSpan.m_traceId = spanId;
  livingSpan.m_spanId = std::move(spanId);
  livingSpan.m_alive = true;
  livingSpan.m_startTime = startTime;

  return livingSpan;
}

// -----------------------------------------------------------------------------

server::LivingSpan server::LivingSpan::Create(const Span& parentSpan)
{
  std::chrono::steady_clock::time_point startTime
    = std::chrono::steady_clock::now();

  LivingSpan livingSpan;

  std::string spanId = GenerateSpanId(startTime);

  livingSpan.m_traceId = parentSpan.m_traceId;
  livingSpan.m_spanId = std::move(spanId);
  livingSpan.m_parentSpanId = parentSpan.m_spanId;
  livingSpan.m_alive = true;
  livingSpan.m_startTime = startTime;

  return livingSpan;
}

// -----------------------------------------------------------------------------

server::LivingSpan server::LivingSpan::Create(
  const Span& parentSpan,
  const std::vector<std::string>& tags)
{
  std::chrono::steady_clock::time_point startTime
    = std::chrono::steady_clock::now();

  LivingSpan livingSpan;

  std::string spanId = GenerateSpanId(startTime, tags);

  livingSpan.m_traceId = parentSpan.m_traceId;
  livingSpan.m_spanId = std::move(spanId);
  livingSpan.m_parentSpanId = parentSpan.m_spanId;
  livingSpan.m_alive = true;
  livingSpan.m_startTime = startTime;

  return livingSpan;
}

// -----------------------------------------------------------------------------

server::LivingSpan::LivingSpan()
  : Span()
{
}

// -----------------------------------------------------------------------------

server::LivingSpan::LivingSpan(LivingSpan&& src)
  : Span(std::move(src))
  , m_alive(src.m_alive)
  , m_startTime(src.m_startTime)
{
  src.m_alive = false;
}

// -----------------------------------------------------------------------------

server::LivingSpan::~LivingSpan()
{
  if (m_alive)
  {
    std::chrono::steady_clock::time_point finishTime
      = std::chrono::steady_clock::now();
    std::stringstream ss;
    ss
      << "<span>"
      << " trace_id: " << m_traceId
      << ", span_id: " << m_spanId;
    if (m_parentSpanId)
    {
      ss << ", parent_span_id: " << *m_parentSpanId;
    }
    ss
      << ", start_time: "
      << m_startTime.time_since_epoch().count()
      << ", finish_time: "
      << finishTime.time_since_epoch().count();
    LOG_SPAN_TIME(*this, finishTime, server::LogSeverity::DEBUG, ss.str());
  }
}

// -----------------------------------------------------------------------------

server::LivingSpan& server::LivingSpan::operator=(LivingSpan&& src)
{
  Span::operator=(std::move(src));
  m_alive = src.m_alive;
  m_startTime = src.m_startTime;

  src.m_alive = false;

  return *this;
}

// -----------------------------------------------------------------------------

static std::string GenerateProcessName()
{
#if defined(PLATFORM_POSIX)

  char hostname[HOST_NAME_MAX + 1] = { 0 };
  gethostname(hostname, HOST_NAME_MAX);
  return std::string{ hostname };

#elif defined(PLATFORM_WINDOWS)

  char hostname[MAX_COMPUTERNAME_LENGTH + 1];
  unsigned long hostnameLength = MAX_COMPUTERNAME_LENGTH + 1;
  if (GetComputerNameA(hostname, &hostnameLength))
  {
    return hostname;
  }
  return "process";

#else

  return "process";

#endif
}

// -----------------------------------------------------------------------------

static std::string GenerateSpanId(std::chrono::steady_clock::time_point time)
{
  std::stringstream ss;
  ss << s_processName;
  ss << '-' << time.time_since_epoch().count();
  ss << '-' << std::setw(4) << std::setfill('0') << std::hex << s_salt++;
  return ss.str();
}

// -----------------------------------------------------------------------------

static std::string GenerateSpanId(
  std::chrono::steady_clock::time_point time,
  const std::vector<std::string>& tags)
{
  std::stringstream ss;
  ss << s_processName;
  for (const std::string& tag : tags)
  {
    ss << '-' << tag;
  }
  ss << '-' << time.time_since_epoch().count();
  ss << '-' << std::setw(4) << std::setfill('0') << std::hex << s_salt++;
  return ss.str();
}

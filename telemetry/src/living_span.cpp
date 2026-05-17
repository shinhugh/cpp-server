#include "living_span.h"

#include "span.h"
#include "trace.h"

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

// -----------------------------------------------------------------------------

static std::string GenerateProcessName();
static std::string GenerateSpanId(
    std::chrono::steady_clock::time_point time);
static std::string GenerateSpanId(
    std::chrono::steady_clock::time_point time, const std::string &tag);

// -----------------------------------------------------------------------------

static const std::string s_processName = GenerateProcessName();
static std::atomic<uint16_t> s_salt = 0;

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    bool openOnCreate)
    : m_invalid(false),
      m_open(false)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const std::string &tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_tag(tag)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    std::string &&tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_tag(std::move(tag))
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const char *tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_tag(tag)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const Span &parentSpan, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_parentSpan(parentSpan)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const Span &parentSpan, const std::string &tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_parentSpan(parentSpan),
      m_tag(tag)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const Span &parentSpan, std::string &&tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_parentSpan(parentSpan),
      m_tag(std::move(tag))
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    const Span &parentSpan, const char *tag, bool openOnCreate)
    : m_invalid(false),
      m_open(false),
      m_parentSpan(parentSpan),
      m_tag(tag)
{
  if (openOnCreate)
  {
    Open();
  }
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::LivingSpan(
    LivingSpan &&src)
{
  m_invalid = src.m_invalid;
  src.m_invalid = true;
  m_open = src.m_open;
  src.m_open = false;
  m_parentSpan = std::move(src.m_parentSpan);
  m_tag = std::move(src.m_tag);
  m_traceId = std::move(src.m_traceId);
  m_spanId = std::move(src.m_spanId);
  m_parentSpanId = std::move(src.m_parentSpanId);
  m_startTime = src.m_startTime;
  m_finishTime = src.m_finishTime;
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan::~LivingSpan()
{
  Close();
}

// -----------------------------------------------------------------------------

telemetry::LivingSpan &telemetry::LivingSpan::operator=(
    LivingSpan &&src)
{
  m_invalid = src.m_invalid;
  src.m_invalid = true;
  m_open = src.m_open;
  src.m_open = false;
  m_parentSpan = std::move(src.m_parentSpan);
  m_tag = std::move(src.m_tag);
  m_traceId = std::move(src.m_traceId);
  m_spanId = std::move(src.m_spanId);
  m_parentSpanId = std::move(src.m_parentSpanId);
  m_startTime = src.m_startTime;
  m_finishTime = src.m_finishTime;

  return *this;
}

// -----------------------------------------------------------------------------

void telemetry::LivingSpan::Open()
{
  Open(std::chrono::steady_clock::now());
}

// -----------------------------------------------------------------------------

void telemetry::LivingSpan::Open(
    std::chrono::steady_clock::time_point startTime)
{
  if (m_invalid || m_open)
  {
    return;
  }

  m_open = true;
  m_startTime = startTime;
  m_spanId =
      m_tag ? GenerateSpanId(m_startTime, *m_tag) : GenerateSpanId(m_startTime);
  if (m_parentSpan)
  {
    m_traceId = m_parentSpan->m_traceId;
    m_parentSpanId = m_parentSpan->m_spanId;
  }
  else
  {
    m_traceId = m_spanId;
  }
}

// -----------------------------------------------------------------------------

void telemetry::LivingSpan::Close()
{
  Close(std::chrono::steady_clock::now());
}

// -----------------------------------------------------------------------------

void telemetry::LivingSpan::Close(
    std::chrono::steady_clock::time_point finishTime)
{
  if (m_invalid || !m_open)
  {
    return;
  }

  m_finishTime = finishTime;
  if (m_finishTime < m_startTime)
  {
    m_finishTime = m_startTime;
  }
  m_invalid = true;
  m_open = false;

  impl::CommitSpan(*this);
}

// -----------------------------------------------------------------------------

static std::string GenerateProcessName()
{
#if defined(PLATFORM_POSIX)

  char hostname[HOST_NAME_MAX + 1] = {0};
  gethostname(hostname, HOST_NAME_MAX);
  return std::string{hostname};

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

static std::string GenerateSpanId(
    std::chrono::steady_clock::time_point time)
{
  std::stringstream ss;
  ss << s_processName << '-' << time.time_since_epoch().count() << '-'
     << std::setw(4) << std::setfill('0') << std::hex << s_salt++;
  return ss.str();
}

// -----------------------------------------------------------------------------

static std::string GenerateSpanId(
    std::chrono::steady_clock::time_point time, const std::string &tag)
{
  std::stringstream ss;
  ss << s_processName << '-' << tag << '-' << time.time_since_epoch().count()
     << '-' << std::setw(4) << std::setfill('0') << std::hex << s_salt++;
  return ss.str();
}

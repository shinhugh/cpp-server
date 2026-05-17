#include "async/async.h"
#include "async/future.h"
#include "telemetry/living_span.h"
#include "telemetry/log.h"
#include "telemetry/log_severity.h"
#include "telemetry/log_value.h"
#include "telemetry/span.h"
#include "telemetry/trace.h"

#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

#define FLUSH_SPANS_INTERVAL_MS 250
#define FLUSH_LOGS_INTERVAL_MS 200

// -----------------------------------------------------------------------------

static int Application(
    int argc, char *argv[]);

static void WriteSpanToStdout(
    const telemetry::Span &);

static void WriteLogToStdout(
    const telemetry::Span &, std::chrono::system_clock::time_point,
    telemetry::LogSeverity, const std::string &event,
    const std::vector<std::pair<std::string, telemetry::LogValue>> &fields);

static void SerializeLogValue(
    std::ostream &, const telemetry::LogValue &);

// -----------------------------------------------------------------------------

int main(
    int argc, char *argv[])
{
  return async::RunApplication(Application, argc, argv);
}

// -----------------------------------------------------------------------------

static int Application(
    int, char *[])
{
  telemetry::LivingSpan span{"origin"};

  telemetry::RegisterSpanHandler(WriteSpanToStdout);
  telemetry::RegisterLogHandler(WriteLogToStdout);

  std::vector<async::Future<void>> telemetryPublicationTasks;

  bool quitTelemetry = false;

  telemetryPublicationTasks.push_back(
      async::RunOnNewCoroutine<void>(
          [
              &quitTelemetry]()
          {
            while (!quitTelemetry)
            {
              std::chrono::steady_clock::time_point startTime =
                  std::chrono::steady_clock::now();
              telemetry::FlushSpans();
              async::YieldUntil(
                  startTime +
                  std::chrono::milliseconds(FLUSH_SPANS_INTERVAL_MS));
            }
            telemetry::FlushSpans();
          }));

  telemetryPublicationTasks.push_back(
      async::RunOnNewCoroutine<void>(
          [
              &quitTelemetry]()
          {
            while (!quitTelemetry)
            {
              std::chrono::steady_clock::time_point startTime =
                  std::chrono::steady_clock::now();
              telemetry::FlushLogs();
              async::YieldUntil(
                  startTime +
                  std::chrono::milliseconds(FLUSH_LOGS_INTERVAL_MS));
            }
            telemetry::FlushLogs();
          }));

  // Execute application logic here

  span.Close();

  quitTelemetry = true;

  async::Future<void>::RequireAll(telemetryPublicationTasks).Await();

  return 0;
}

// -----------------------------------------------------------------------------

static void WriteSpanToStdout(
    const telemetry::Span &span)
{
  std::stringstream ss;
  ss << "span:" << std::endl
     << "  trace ID:       " << span.m_traceId << std::endl
     << "  span ID:        " << span.m_spanId << std::endl
     << "  parent span ID: "
     << (span.m_parentSpanId ? *span.m_parentSpanId : "<none>") << std::endl;
  std::cout << ss.str();
}

// -----------------------------------------------------------------------------

static void WriteLogToStdout(
    const telemetry::Span &span, std::chrono::system_clock::time_point time,
    telemetry::LogSeverity severity, const std::string &event,
    const std::vector<std::pair<std::string, telemetry::LogValue>> &fields)
{
  std::time_t tt = std::chrono::system_clock::to_time_t(time);
  std::tm localtime = *std::localtime(&tt);

  std::ostringstream os;
  os << std::left << '[' << std::put_time(&localtime, "%F %T") << ']'
     << std::endl
     << '[' << span.m_traceId << "] [" << span.m_spanId << ']' << std::endl
     << '<' << std::setw(6);
  switch (severity)
  {
    case telemetry::LogSeverity::DEBUG:
      os << "DEBUG>";
      break;
    case telemetry::LogSeverity::INFO:
      os << "INFO>";
      break;
    case telemetry::LogSeverity::WARN:
      os << "WARN>";
      break;
    case telemetry::LogSeverity::ERROR:
      os << "ERROR>";
      break;
    case telemetry::LogSeverity::FATAL:
      os << "FATAL>";
      break;
  }
  os << ' ' << event << std::endl;
  if (!fields.empty())
  {
    os << '{';
    bool addComma = false;
    for (const auto &[k, v] : fields)
    {
      if (addComma)
      {
        os << ',';
      }
      os << ' ' << k << ": ";
      SerializeLogValue(os, v);
      addComma = true;
    }
    os << " }" << std::endl;
  }

  std::cout << os.str();
}

// -----------------------------------------------------------------------------

static void SerializeLogValue(
    std::ostream &os, const telemetry::LogValue &v)
{
  bool addComma = false;
  switch (v.GetType())
  {
    case telemetry::LogValue::Type::VOID:
      os << "<none>";
      break;
    case telemetry::LogValue::Type::BOOLEAN:
      os << (v.GetBoolean() ? "true" : "false");
      break;
    case telemetry::LogValue::Type::INTEGER:
      os << v.GetInteger();
      break;
    case telemetry::LogValue::Type::FLOAT:
      os << v.GetFloat();
      break;
    case telemetry::LogValue::Type::STRING:
      os << v.GetString();
      break;
    case telemetry::LogValue::Type::ARRAY:
      os << '[';
      for (const telemetry::LogValue &value : v.GetArray())
      {
        if (addComma)
        {
          os << ',';
        }
        os << ' ';
        SerializeLogValue(os, value);
        addComma = true;
      }
      os << " ]";
      break;
    case telemetry::LogValue::Type::OBJECT:
      os << '{';
      for (const auto &[key, value] : v.GetObject())
      {
        if (addComma)
        {
          os << ',';
        }
        os << ' ' << key << ": ";
        SerializeLogValue(os, value);
        addComma = true;
      }
      os << " }";
      break;
  }
}

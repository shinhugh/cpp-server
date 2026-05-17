#include "trace.h"

#include "span.h"

#include <functional>
#include <mutex>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

static std::vector<telemetry::Span> s_spans;
static std::mutex s_spansMutex;
static std::vector<std::function<void(const telemetry::Span &)>> s_spanHandlers;
static std::mutex s_spanHandlersMutex;

// -----------------------------------------------------------------------------

void telemetry::FlushSpans()
{
  std::vector<Span> spans;
  {
    std::lock_guard lock{s_spansMutex};
    spans.swap(s_spans);
  }

  {
    std::lock_guard lock{s_spanHandlersMutex};
    for (Span &span : spans)
    {
      for (std::function<void(const Span &)> &spanHandler : s_spanHandlers)
      {
        spanHandler(span);
      }
    }
  }
}

// -----------------------------------------------------------------------------

void telemetry::RegisterSpanHandler(
    std::function<void(const Span &)> &&spanHandler)
{
  std::lock_guard lock{s_spanHandlersMutex};
  s_spanHandlers.push_back(std::move(spanHandler));
}

// -----------------------------------------------------------------------------

void telemetry::impl::CommitSpan(
    const Span &span)
{
  std::lock_guard lock{s_spansMutex};
  s_spans.push_back(span);
}

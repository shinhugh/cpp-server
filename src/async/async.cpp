#include "async.h"

#include "async_manager.h"
#include "future.h"
#include "thread_local_state.h"

#include "telemetry/span.h"

#include <functional>
#include <optional>
#include <utility>

// -----------------------------------------------------------------------------

void server::InitializeProcessSpan()
{
  impl::InitializeProcessSpan();
}

// -----------------------------------------------------------------------------

void server::TerminateProcessSpan()
{
  impl::TerminateProcessSpan();
}

// -----------------------------------------------------------------------------

const server::Span& server::GetActiveSpan()
{
  return impl::GetActiveSpan();
}

// -----------------------------------------------------------------------------

template <>
server::Future<void> server::RunTaskOnNewThread(std::function<void()>&& task)
{
  return g_asyncManager->RunTaskOnNewThread(std::move(task));
}

// -----------------------------------------------------------------------------

template <>
server::Future<void> server::RunTaskOnNewCoroutine(std::function<void()>&& task)
{
  return g_asyncManager->RunTaskOnNewCoroutine(std::move(task));
}

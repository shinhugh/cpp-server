#pragma once

#include "future.h"

#include "telemetry/span.h"

#include <functional>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

void InitializeProcessSpan();
void TerminateProcessSpan();
const Span& GetActiveSpan();
template <typename T>
Future<T> RunTaskOnNewThread(std::function<T()>&&);
template <>
Future<void> RunTaskOnNewThread(std::function<void()>&&);
template <typename T>
Future<T> RunTaskOnNewCoroutine(std::function<T()>&&);
template <>
Future<void> RunTaskOnNewCoroutine(std::function<void()>&&);

// -----------------------------------------------------------------------------

} // server

// -----------------------------------------------------------------------------

#include "async_manager.h"

#include <optional>
#include <utility>

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::RunTaskOnNewThread(std::function<T()>&& task)
{
  return g_asyncManager->RunTaskOnNewThread(std::move(task));
}

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::RunTaskOnNewCoroutine(std::function<T()>&& task)
{
  return g_asyncManager->RunTaskOnNewCoroutine(std::move(task));
}

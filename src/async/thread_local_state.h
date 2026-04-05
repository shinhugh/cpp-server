#pragma once

#include "telemetry/living_span.h"
#include "telemetry/span.h"

#include <functional>
#include <memory>

// -----------------------------------------------------------------------------

namespace server::impl
{

// -----------------------------------------------------------------------------

struct ThreadLocalCoroutineContext
{
  std::function<void()> m_requeueCallback;
  std::function<void()> m_yieldCallback;
  std::unique_ptr<LivingSpan> m_span;
};

// -----------------------------------------------------------------------------

void InitializeProcessSpan();
void TerminateProcessSpan();
const Span& GetActiveSpan();
ThreadLocalCoroutineContext* CreateThreadLocalCoroutineContext();
ThreadLocalCoroutineContext* GetThreadLocalCoroutineContext();
void DestroyThreadLocalCoroutineContext();

// -----------------------------------------------------------------------------

} // server::impl

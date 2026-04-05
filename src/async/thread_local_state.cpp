#include "thread_local_state.h"

#include "telemetry/living_span.h"
#include "telemetry/span.h"

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

static std::unordered_map<
  std::thread::id,
  server::impl::ThreadLocalCoroutineContext>
  s_threadLocalCoroutineContexts;
static std::mutex s_threadLocalCoroutineContextsMutex;
static std::optional<server::LivingSpan> s_processSpan;

// -----------------------------------------------------------------------------

void server::impl::InitializeProcessSpan()
{
  s_processSpan
    = server::LivingSpan::Create(std::vector{ std::string{ "LOCAL" } });
}

// -----------------------------------------------------------------------------

void server::impl::TerminateProcessSpan()
{
  s_processSpan.reset();
}

// -----------------------------------------------------------------------------

const server::Span& server::impl::GetActiveSpan()
{
  ThreadLocalCoroutineContext* context = GetThreadLocalCoroutineContext();

  if (context)
  {
    return *context->m_span;
  }

  return *s_processSpan;
}

// -----------------------------------------------------------------------------

server::impl::ThreadLocalCoroutineContext*
  server::impl::CreateThreadLocalCoroutineContext()
{
  std::lock_guard lock{ s_threadLocalCoroutineContextsMutex };
  return &s_threadLocalCoroutineContexts[std::this_thread::get_id()];
}

// -----------------------------------------------------------------------------

server::impl::ThreadLocalCoroutineContext*
  server::impl::GetThreadLocalCoroutineContext()
{
  std::lock_guard lock{ s_threadLocalCoroutineContextsMutex };
  auto it = s_threadLocalCoroutineContexts.find(std::this_thread::get_id());
  if (it != s_threadLocalCoroutineContexts.end())
  {
    return &it->second;
  }
  return nullptr;
}

// -----------------------------------------------------------------------------

void server::impl::DestroyThreadLocalCoroutineContext()
{
  std::lock_guard lock{ s_threadLocalCoroutineContextsMutex };
  s_threadLocalCoroutineContexts.erase(std::this_thread::get_id());
}

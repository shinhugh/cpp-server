#pragma once

#include <functional>
#include <mutex>
#include <optional>
#include <vector>

// -----------------------------------------------------------------------------

namespace server::impl
{

// -----------------------------------------------------------------------------

template <typename T>
class FutureState
{
public:
  void Fulfill(T&&);
  const T& Await();

private:
  std::mutex m_mutex;
  bool m_fulfilled = false;
  std::optional<T> m_result;
  std::vector<std::function<void()>> m_onFulfillCallbacks;
};

// -----------------------------------------------------------------------------

template <>
class FutureState<void>
{
public:
  void Fulfill();
  void Await();

private:
  std::mutex m_mutex;
  bool m_fulfilled = false;
  std::vector<std::function<void()>> m_onFulfillCallbacks;
};

// -----------------------------------------------------------------------------

} // server::impl

// -----------------------------------------------------------------------------

#include "thread_local_state.h"

#include "telemetry/living_span.h"

#include <condition_variable>
#include <memory>
#include <utility>

// -----------------------------------------------------------------------------

template <typename T>
void server::impl::FutureState<T>::Fulfill(T&& result)
{
  std::vector<std::function<void()>> onFulfillCallbacks;
  {
    std::lock_guard lock{ m_mutex };
    m_result = std::move(result);
    m_fulfilled = true;
    onFulfillCallbacks.swap(m_onFulfillCallbacks);
  }

  for (std::function<void()>& onFulfillCallback : onFulfillCallbacks)
  {
    onFulfillCallback();
  }
}

// -----------------------------------------------------------------------------

template <typename T>
const T& server::impl::FutureState<T>::Await()
{
  ThreadLocalCoroutineContext* context = GetThreadLocalCoroutineContext();

  std::unique_lock futureLock{ m_mutex };

  if (m_fulfilled)
  {
    return *m_result;
  }

  if (!context)
  {
    bool threadReady = false;
    std::mutex threadReadyMutex;
    std::condition_variable threadReadyCv;

    m_onFulfillCallbacks.push_back([
      &threadReady,
      &threadReadyMutex,
      &threadReadyCv]()
      {
        {
          std::lock_guard threadReadyLock{ threadReadyMutex };
          threadReady = true;
        }
        threadReadyCv.notify_one();
      });

    futureLock.unlock();

    std::unique_lock threadReadyLock{ threadReadyMutex };
    while (!threadReady)
    {
      threadReadyCv.wait(threadReadyLock);
    }
  }

  else
  {
    m_onFulfillCallbacks.push_back(std::move(context->m_requeueCallback));

    futureLock.unlock();

    std::unique_ptr<LivingSpan> span{ std::move(context->m_span) };
    context->m_span.reset();
    std::function<void()> yieldCallback = std::move(context->m_yieldCallback);
    context->m_yieldCallback = {};

    yieldCallback();

    context->m_yieldCallback = std::move(yieldCallback);
    context->m_span = std::move(span);
  }

  return *m_result;
}

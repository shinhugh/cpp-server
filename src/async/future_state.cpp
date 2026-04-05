#include "future_state.h"

#include "thread_local_state.h"

#include "telemetry/living_span.h"

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

void server::impl::FutureState<void>::Fulfill()
{
  std::vector<std::function<void()>> onFulfillCallbacks;
  {
    std::lock_guard lock{ m_mutex };
    m_fulfilled = true;
    onFulfillCallbacks.swap(m_onFulfillCallbacks);
  }

  for (std::function<void()>& onFulfillCallback : onFulfillCallbacks)
  {
    onFulfillCallback();
  }
}

// -----------------------------------------------------------------------------

void server::impl::FutureState<void>::Await()
{
  ThreadLocalCoroutineContext* context = GetThreadLocalCoroutineContext();

  std::unique_lock futureLock{ m_mutex };

  if (m_fulfilled)
  {
    return;
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
}

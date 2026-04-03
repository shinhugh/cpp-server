#pragma once

#include "future.h"

#include "subsystem/manager.h"

#include <boost/context/continuation_fcontext.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

class AsyncManager : public Manager
{
public:
  virtual void Process() override;
  template <typename T>
  Future<T> RunTaskOnNewThread(std::function<T()>&&);
  Future<void> RunTaskOnNewThread(std::function<void()>&&);
  size_t DestroyCompleteThreads();
  template <typename T>
  Future<T> RunTaskOnNewCoroutine(std::function<T()>&&);
  Future<void> RunTaskOnNewCoroutine(std::function<void()>&&);
  size_t RunReadyCoroutines();

private:
  std::unordered_map<std::thread::id, std::thread> m_activeThreads;
  std::mutex m_activeThreadsMutex;
  std::vector<std::thread> m_completeThreads;
  std::mutex m_completeThreadsMutex;
  std::vector<std::shared_ptr<boost::context::continuation>>
    m_readyContinuations;
  std::mutex m_readyContinuationsMutex;
};

// -----------------------------------------------------------------------------

extern std::optional<AsyncManager> g_asyncManager;

// -----------------------------------------------------------------------------

} // server

// -----------------------------------------------------------------------------

#include "future_state.h"
#include "thread_local_state.h"

#include "telemetry/living_span.h"
#include "telemetry/span.h"

#include <condition_variable>
#include <utility>

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::AsyncManager::RunTaskOnNewThread(
  std::function<T()>&& task)
{
  std::shared_ptr<impl::FutureState<T>> future
    = std::make_shared<impl::FutureState<T>>();

  std::shared_ptr<bool> threadAddedToActivePool = std::make_shared<bool>(false);
  std::shared_ptr<std::mutex> threadAddedToActivePoolMutex
    = std::make_shared<std::mutex>();
  std::shared_ptr<std::condition_variable> threadAddedToActivePoolCv
    = std::make_shared<std::condition_variable>();

  std::thread childThread{ [
    this,
    task = std::move(task),
    future,
    threadAddedToActivePool,
    threadAddedToActivePoolMutex,
    threadAddedToActivePoolCv]()
    {
      future->Fulfill(task());

      {
        std::unique_lock lock{ *threadAddedToActivePoolMutex };
        while (!*threadAddedToActivePool)
        {
          threadAddedToActivePoolCv->wait(lock);
        }
      }

      std::thread completeThread;
      {
        std::lock_guard lock{ m_activeThreadsMutex };
        completeThread
          = std::move(m_activeThreads.at(std::this_thread::get_id()));
        m_activeThreads.erase(std::this_thread::get_id());
      }
      std::lock_guard lock{ m_completeThreadsMutex };
      m_completeThreads.push_back(std::move(completeThread));
    } };

  {
    std::lock_guard lock{ m_activeThreadsMutex };
    m_activeThreads.emplace(childThread.get_id(), std::move(childThread));
  }

  {
    std::lock_guard lock{ *threadAddedToActivePoolMutex };
    *threadAddedToActivePool = true;
  }
  threadAddedToActivePoolCv->notify_one();

  return Future<T>{ future };
}

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::AsyncManager::RunTaskOnNewCoroutine(
  std::function<T()>&& task)
{
  std::shared_ptr<impl::FutureState<T>> future
    = std::make_shared<impl::FutureState<T>>();

  boost::context::continuation childContext = boost::context::callcc(
    [task = std::move(task), future, span = impl::GetActiveSpan()](
      boost::context::continuation&& parentContinuation)
    {
      parentContinuation = parentContinuation.resume();

      impl::ThreadLocalCoroutineContext* context
        = impl::GetThreadLocalCoroutineContext();

      context->m_yieldCallback = [&parentContinuation]()
        {
          parentContinuation = parentContinuation.resume();
        };
      context->m_span = std::make_unique<LivingSpan>(LivingSpan::Create(span));

      T result = task();

      context->m_span.reset();
      context->m_yieldCallback = {};

      future->Fulfill(std::move(result));

      return std::move(parentContinuation);
    });

  {
    std::lock_guard lock{ m_readyContinuationsMutex };
    m_readyContinuations.push_back(
      std::make_shared<boost::context::continuation>(std::move(childContext)));
  }

  return Future<T>{ future };
}

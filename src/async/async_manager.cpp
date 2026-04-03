#include "async_manager.h"

#include "future_state.h"
#include "future.h"
#include "thread_local_state.h"

#include "telemetry/living_span.h"
#include "telemetry/span.h"

#include <boost/context/continuation_fcontext.hpp>

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

std::optional<server::AsyncManager> server::g_asyncManager;

// -----------------------------------------------------------------------------

void server::AsyncManager::Process()
{
  DestroyCompleteThreads();
  RunReadyCoroutines();
}

// -----------------------------------------------------------------------------

server::Future<void> server::AsyncManager::RunTaskOnNewThread(
  std::function<void()>&& task)
{
  std::shared_ptr<impl::FutureState<void>> future
    = std::make_shared<impl::FutureState<void>>();

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
      task();
      future->Fulfill();

      {
        std::unique_lock lock{ *threadAddedToActivePoolMutex };
        while (!*threadAddedToActivePool)
        {
          threadAddedToActivePoolCv->wait(lock);
        }
      }

      // TODO: It's possible that this thread outlives this AsyncManager, in
      //       which case the member variables accessed here are no longer
      //       valid.
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

  return Future<void>{ future };
}

// -----------------------------------------------------------------------------

size_t server::AsyncManager::DestroyCompleteThreads()
{
  std::vector<std::thread> completeThreads;
  {
    std::lock_guard lock{ m_completeThreadsMutex };
    completeThreads.swap(m_completeThreads);
  }

  for (std::thread& thread : completeThreads)
  {
    thread.join();
  }

  return completeThreads.size();
}

// -----------------------------------------------------------------------------

server::Future<void> server::AsyncManager::RunTaskOnNewCoroutine(
  std::function<void()>&& task)
{
  std::shared_ptr<impl::FutureState<void>> future
    = std::make_shared<impl::FutureState<void>>();

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

      task();

      context->m_span.reset();
      context->m_yieldCallback = {};

      future->Fulfill();

      return std::move(parentContinuation);
    });

  {
    std::lock_guard lock{ m_readyContinuationsMutex };
    m_readyContinuations.push_back(
      std::make_shared<boost::context::continuation>(std::move(childContext)));
  }

  return Future<void>{ future };
}

// -----------------------------------------------------------------------------

size_t server::AsyncManager::RunReadyCoroutines()
{
  std::vector<std::shared_ptr<boost::context::continuation>> readyContinuations;
  {
    std::lock_guard lock{ m_readyContinuationsMutex };
    readyContinuations.swap(m_readyContinuations);
  }

  impl::ThreadLocalCoroutineContext* context
    = impl::CreateThreadLocalCoroutineContext();

  for (std::shared_ptr<boost::context::continuation>& readyContinuation
    : readyContinuations)
  {
    // TODO: It's possible that m_requeueCallback gets invoked and this
    //       continuation is resumed by another thread before this thread yields
    //       this session and updates the continuation.
    // TODO: It's possible that m_requeueCallback gets invoked after this
    //       AsyncManager has already been destructed.
    //       Add logic to AsyncManager::~AsyncManager() that closes every
    //       pending continuation and invalidates all requeue callbacks that
    //       each continuation has registered into FutureState instances.
    context->m_requeueCallback = [this, readyContinuation]()
      {
        std::lock_guard lock{ m_readyContinuationsMutex };
        m_readyContinuations.push_back(readyContinuation);
      };

    *readyContinuation = readyContinuation->resume();
  }

  impl::DestroyThreadLocalCoroutineContext();

  return readyContinuations.size();
}

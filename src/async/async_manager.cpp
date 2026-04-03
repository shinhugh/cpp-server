#include "async_manager.h"

#include "future_state.h"
#include "future.h"

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

  // TODO

  return Future<void>{ future };
}

// -----------------------------------------------------------------------------

size_t server::AsyncManager::RunReadyCoroutines()
{
  // TODO
  return 0;
}

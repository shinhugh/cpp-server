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

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::AsyncManager::RunTaskOnNewThread(
  std::function<T()>&& task)
{
  std::shared_ptr<impl::FutureState<T>> future
    = std::make_shared<impl::FutureState<T>>();

  // TODO

  return Future<T>{ future };
}

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T> server::AsyncManager::RunTaskOnNewCoroutine(
  std::function<T()>&& task)
{
  std::shared_ptr<impl::FutureState<T>> future
    = std::make_shared<impl::FutureState<T>>();

  // TODO

  return Future<T>{ future };
}

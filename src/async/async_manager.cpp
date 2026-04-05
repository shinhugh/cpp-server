#include "async_manager.h"

#include "future_state.h"
#include "future.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

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

  // TODO

  return Future<void>{ future };
}

// -----------------------------------------------------------------------------

size_t server::AsyncManager::DestroyCompleteThreads()
{
  // TODO
  return 0;
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

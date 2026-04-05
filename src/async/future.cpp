#include "future.h"

#include "future_state.h"

#include <memory>

// -----------------------------------------------------------------------------

server::Future<void>::Future(
  const std::shared_ptr<impl::FutureState<void>>& coroutineFuture)
  : m_futureState(coroutineFuture)
{
}

// -----------------------------------------------------------------------------

void server::Future<void>::Await()
{
  m_futureState->Await();
}

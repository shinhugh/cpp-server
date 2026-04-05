#pragma once

#include "future_state.h"

#include <memory>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

template <typename T>
class Future
{
public:
  Future(const std::shared_ptr<impl::FutureState<T>>&);
  const T& Await();

private:
  std::shared_ptr<impl::FutureState<T>> m_futureState;
};

// -----------------------------------------------------------------------------

template <>
class Future<void>
{
public:
  Future(const std::shared_ptr<impl::FutureState<void>>&);
  void Await();

private:
  std::shared_ptr<impl::FutureState<void>> m_futureState;
};

// -----------------------------------------------------------------------------

} // server

// -----------------------------------------------------------------------------

template <typename T>
server::Future<T>::Future(
  const std::shared_ptr<impl::FutureState<T>>& coroutineFuture)
  : m_futureState(coroutineFuture)
{
}

// -----------------------------------------------------------------------------

template <typename T>
const T& server::Future<T>::Await()
{
  return m_futureState->Await();
}

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

// TODO: #includes

// -----------------------------------------------------------------------------

template <typename T>
void server::impl::FutureState<T>::Fulfill(T&& result)
{
  // TODO
}

// -----------------------------------------------------------------------------

template <typename T>
const T& server::impl::FutureState<T>::Await()
{
  // TODO
  return *m_result;
}

#pragma once

#include "log_severity.h"
#include "log_value.h"
#include "span.h"

#include <chrono>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

namespace telemetry
{

// -----------------------------------------------------------------------------

template <typename... Fields>
void Log(
    const Span &, std::chrono::system_clock::time_point, LogSeverity,
    const std::string &event, Fields &&...);

template <typename... Fields>
void Log(
    const Span &, std::chrono::system_clock::time_point, LogSeverity,
    std::string &&event, Fields &&...);

template <typename... Fields>
void Log(
    Span &&, std::chrono::system_clock::time_point, LogSeverity,
    const std::string &event, Fields &&...);

template <typename... Fields>
void Log(
    Span &&, std::chrono::system_clock::time_point, LogSeverity,
    std::string &&event, Fields &&...);

template <typename T>
std::pair<std::string, LogValue> Field(
    const std::string &, const T &);

template <
    typename T,
    typename std::enable_if<!std::is_reference<T>::value, bool>::type = true>
std::pair<std::string, LogValue> Field(
    const std::string &, T &&);

template <typename T>
std::pair<std::string, LogValue> Field(
    std::string &&, const T &);

template <
    typename T,
    typename std::enable_if<!std::is_reference<T>::value, bool>::type = true>
std::pair<std::string, LogValue> Field(
    std::string &&, T &&);

LogValue Capture();

LogValue Capture(
    bool);

LogValue Capture(
    long long);

LogValue Capture(
    double);

LogValue Capture(
    std::string &&);

LogValue Capture(
    std::vector<LogValue> &&);

LogValue Capture(
    std::unordered_map<std::string, LogValue> &&);

LogValue Capture(
    int);

LogValue Capture(
    unsigned int);

LogValue Capture(
    long);

LogValue Capture(
    unsigned long);

LogValue Capture(
    float);

LogValue Capture(
    const std::string &);

LogValue Capture(
    const char *);

LogValue Capture(
    const void *);

template <typename T>
LogValue Capture(
    const T *);

template <typename T>
LogValue Capture(
    const std::optional<T> &);

template <typename T>
LogValue Capture(
    std::optional<T> &&);

template <typename T>
LogValue Capture(
    const std::unique_ptr<T> &);

template <typename T>
LogValue Capture(
    std::unique_ptr<T> &&);

template <typename T>
LogValue Capture(
    const std::shared_ptr<T> &);

template <typename T>
LogValue Capture(
    std::shared_ptr<T> &&);

template <typename T1, typename T2>
LogValue Capture(
    const std::pair<T1, T2> &);

template <typename T1, typename T2>
LogValue Capture(
    std::pair<T1, T2> &&);

template <typename... Ts>
LogValue Capture(
    const std::tuple<Ts...> &);

template <typename... Ts>
LogValue Capture(
    std::tuple<Ts...> &&);

template <typename T>
LogValue Capture(
    const std::vector<T> &);

template <typename T>
LogValue Capture(
    std::vector<T> &&);

template <typename T>
LogValue Capture(
    const std::unordered_set<T> &);

template <typename T>
LogValue Capture(
    std::unordered_set<T> &&);

template <typename T>
LogValue Capture(
    const std::set<T> &);

template <typename T>
LogValue Capture(
    std::set<T> &&);

template <typename K, typename V>
LogValue Capture(
    const std::unordered_map<K, V> &);

template <typename K, typename V>
LogValue Capture(
    std::unordered_map<K, V> &&);

template <typename T>
LogValue Capture(
    const std::unordered_map<std::string, T> &);

template <typename T>
LogValue Capture(
    std::unordered_map<std::string, T> &&);

template <typename K, typename V>
LogValue Capture(
    const std::map<K, V> &);

template <typename K, typename V>
LogValue Capture(
    std::map<K, V> &&);

template <typename T>
LogValue Capture(
    const std::map<std::string, T> &);

template <typename T>
LogValue Capture(
    std::map<std::string, T> &&);

void FlushLogs();

void RegisterLogHandler(
    std::function<void(
        const Span &, std::chrono::system_clock::time_point, LogSeverity,
        const std::string &,
        const std::vector<std::pair<std::string, LogValue>> &)> &&);

// -----------------------------------------------------------------------------

}  // namespace telemetry

// -----------------------------------------------------------------------------

#include <mutex>

// -----------------------------------------------------------------------------

namespace telemetry::impl
{

// -----------------------------------------------------------------------------

struct LogEntry
{
  Span m_span;
  std::chrono::system_clock::time_point m_time;
  LogSeverity m_severity;
  std::string m_event;
  std::vector<std::pair<std::string, LogValue>> m_fields;
};

// -----------------------------------------------------------------------------

void AddFieldsToList(
    std::vector<std::pair<std::string, LogValue>> &);

template <typename... Fields>
void AddFieldsToList(
    std::vector<std::pair<std::string, LogValue>> &,
    std::pair<std::string, LogValue> &&, Fields &&...fields);

template <
    size_t S, typename... Ts,
    typename std::enable_if<S == 0, bool>::type = true>
void AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, const std::tuple<Ts...> &);

template <
    size_t S, typename... Ts,
    typename std::enable_if<(S > 0), bool>::type = true>
void AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, const std::tuple<Ts...> &);

template <
    size_t S, typename... Ts,
    typename std::enable_if<S == 0, bool>::type = true>
void AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, std::tuple<Ts...> &&);

template <
    size_t S, typename... Ts,
    typename std::enable_if<(S > 0), bool>::type = true>
void AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, std::tuple<Ts...> &&);

// -----------------------------------------------------------------------------

extern std::vector<LogEntry> g_logEntries;
extern std::mutex g_logEntriesMutex;

// -----------------------------------------------------------------------------

}  // namespace telemetry::impl

// -----------------------------------------------------------------------------

template <typename... Fields>
void telemetry::Log(
    const Span &span, std::chrono::system_clock::time_point time,
    LogSeverity severity, const std::string &event, Fields &&...fields)
{
  std::vector<std::pair<std::string, LogValue>> fieldList;
  impl::AddFieldsToList(fieldList, std::forward<Fields>(fields)...);
  std::lock_guard lock{impl::g_logEntriesMutex};
  impl::g_logEntries.push_back(
      {span, time, severity, event, std::move(fieldList)});
}

// -----------------------------------------------------------------------------

template <typename... Fields>
void telemetry::Log(
    const Span &span, std::chrono::system_clock::time_point time,
    LogSeverity severity, std::string &&event, Fields &&...fields)
{
  std::vector<std::pair<std::string, LogValue>> fieldList;
  impl::AddFieldsToList(fieldList, std::forward<Fields>(fields)...);
  std::lock_guard lock{impl::g_logEntriesMutex};
  impl::g_logEntries.push_back(
      {span, time, severity, std::move(event), std::move(fieldList)});
}

// -----------------------------------------------------------------------------

template <typename... Fields>
void telemetry::Log(
    Span &&span, std::chrono::system_clock::time_point time,
    LogSeverity severity, const std::string &event, Fields &&...fields)
{
  std::vector<std::pair<std::string, LogValue>> fieldList;
  impl::AddFieldsToList(fieldList, std::forward<Fields>(fields)...);
  std::lock_guard lock{impl::g_logEntriesMutex};
  impl::g_logEntries.push_back(
      {std::move(span), time, severity, event, std::move(fieldList)});
}

// -----------------------------------------------------------------------------

template <typename... Fields>
void telemetry::Log(
    Span &&span, std::chrono::system_clock::time_point time,
    LogSeverity severity, std::string &&event, Fields &&...fields)
{
  std::vector<std::pair<std::string, LogValue>> fieldList;
  impl::AddFieldsToList(fieldList, std::forward<Fields>(fields)...);
  std::lock_guard lock{impl::g_logEntriesMutex};
  impl::g_logEntries.push_back(
      {std::move(span), time, severity, std::move(event),
          std::move(fieldList)});
}

// -----------------------------------------------------------------------------

template <typename T>
std::pair<std::string, telemetry::LogValue> telemetry::Field(
    const std::string &k, const T &v)
{
  return std::make_pair(k, Capture(v));
}

// -----------------------------------------------------------------------------

template <
    typename T,
    typename std::enable_if<!std::is_reference<T>::value, bool>::type>
std::pair<std::string, telemetry::LogValue> telemetry::Field(
    const std::string &k, T &&v)
{
  return std::make_pair(k, Capture(std::move(v)));
}

// -----------------------------------------------------------------------------

template <typename T>
std::pair<std::string, telemetry::LogValue> telemetry::Field(
    std::string &&k, const T &v)
{
  return std::make_pair(std::move(k), Capture(v));
}

// -----------------------------------------------------------------------------

template <
    typename T,
    typename std::enable_if<!std::is_reference<T>::value, bool>::type>
std::pair<std::string, telemetry::LogValue> telemetry::Field(
    std::string &&k, T &&v)
{
  return std::make_pair(std::move(k), Capture(std::move(v)));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const T *v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(*v);
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::optional<T> &v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(*v);
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::optional<T> &&v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(std::move(*v));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::unique_ptr<T> &v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(*v);
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::unique_ptr<T> &&v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(std::move(*v));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::shared_ptr<T> &v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(*v);
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::shared_ptr<T> &&v)
{
  if (!v)
  {
    return Capture();
  }
  return Capture(std::move(*v));
}

// -----------------------------------------------------------------------------

template <typename T1, typename T2>
telemetry::LogValue telemetry::Capture(
    const std::pair<T1, T2> &v)
{
  std::vector<LogValue> array;
  array.push_back(Capture(v.first));
  array.push_back(Capture(v.second));
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T1, typename T2>
telemetry::LogValue telemetry::Capture(
    std::pair<T1, T2> &&v)
{
  std::vector<LogValue> array;
  array.push_back(Capture(std::move(v.first)));
  array.push_back(Capture(std::move(v.second)));
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename... Ts>
telemetry::LogValue telemetry::Capture(
    const std::tuple<Ts...> &v)
{
  std::vector<LogValue> array;
  constexpr size_t tupleSize =
      std::tuple_size<typename std::remove_reference<decltype(v)>::type>::value;
  array.reserve(tupleSize);
  impl::AccumulateTupleElementsIntoArray<tupleSize>(array, v);
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename... Ts>
telemetry::LogValue telemetry::Capture(
    std::tuple<Ts...> &&v)
{
  std::vector<LogValue> array;
  constexpr size_t tupleSize =
      std::tuple_size<typename std::remove_reference<decltype(v)>::type>::value;
  array.reserve(tupleSize);
  impl::AccumulateTupleElementsIntoArray<tupleSize>(array, std::move(v));
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::vector<T> &v)
{
  std::vector<LogValue> array;
  for (const T &e : v)
  {
    array.push_back(Capture(e));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::vector<T> &&v)
{
  std::vector<LogValue> array;
  for (T &e : v)
  {
    array.push_back(Capture(std::move(e)));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::unordered_set<T> &v)
{
  std::vector<LogValue> array;
  for (const T &e : v)
  {
    array.push_back(Capture(e));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::unordered_set<T> &&v)
{
  std::vector<LogValue> array;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    array.push_back(Capture(std::move(extractedNode.value())));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::set<T> &v)
{
  std::vector<LogValue> array;
  for (const T &e : v)
  {
    array.push_back(Capture(e));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::set<T> &&v)
{
  std::vector<LogValue> array;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    array.push_back(Capture(std::move(extractedNode.value())));
  }
  return Capture(std::move(array));
}

// -----------------------------------------------------------------------------

template <typename K, typename V>
telemetry::LogValue telemetry::Capture(
    const std::unordered_map<K, V> &v)
{
  std::unordered_map<std::string, LogValue> object;
  for (const auto &[key, value] : v)
  {
    object.emplace(std::string{key}, Capture(value));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename K, typename V>
telemetry::LogValue telemetry::Capture(
    std::unordered_map<K, V> &&v)
{
  std::unordered_map<std::string, LogValue> object;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    object.emplace(
        std::string{std::move(extractedNode.key())},
        Capture(std::move(extractedNode.mapped())));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::unordered_map<std::string, T> &v)
{
  std::unordered_map<std::string, LogValue> object;
  for (const auto &[key, value] : v)
  {
    object.emplace(key, Capture(value));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::unordered_map<std::string, T> &&v)
{
  std::unordered_map<std::string, LogValue> object;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    object.emplace(
        std::move(extractedNode.key()),
        Capture(std::move(extractedNode.mapped())));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename K, typename V>
telemetry::LogValue telemetry::Capture(
    const std::map<K, V> &v)
{
  std::unordered_map<std::string, LogValue> object;
  for (const auto &[key, value] : v)
  {
    object.emplace(std::string{key}, Capture(value));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename K, typename V>
telemetry::LogValue telemetry::Capture(
    std::map<K, V> &&v)
{
  std::unordered_map<std::string, LogValue> object;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    object.emplace(
        std::string{std::move(extractedNode.key())},
        Capture(std::move(extractedNode.mapped())));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    const std::map<std::string, T> &v)
{
  std::unordered_map<std::string, LogValue> object;
  for (const auto &[key, value] : v)
  {
    object.emplace(key, Capture(value));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename T>
telemetry::LogValue telemetry::Capture(
    std::map<std::string, T> &&v)
{
  std::unordered_map<std::string, LogValue> object;
  for (auto iter = v.begin(); iter != v.end(); iter = v.begin())
  {
    auto extractedNode = v.extract(iter);
    object.emplace(
        std::move(extractedNode.key()),
        Capture(std::move(extractedNode.mapped())));
  }
  return Capture(std::move(object));
}

// -----------------------------------------------------------------------------

template <typename... Fields>
void telemetry::impl::AddFieldsToList(
    std::vector<std::pair<std::string, LogValue>> &fieldList,
    std::pair<std::string, LogValue> &&field, Fields &&...fields)
{
  fieldList.push_back(std::move(field));
  AddFieldsToList(fieldList, std::forward<Fields>(fields)...);
}

// -----------------------------------------------------------------------------

template <size_t S, typename... Ts, typename std::enable_if<S == 0, bool>::type>
void telemetry::impl::AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, const std::tuple<Ts...> &)
{
}

// -----------------------------------------------------------------------------

template <
    size_t S, typename... Ts, typename std::enable_if<(S > 0), bool>::type>
void telemetry::impl::AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &dst, const std::tuple<Ts...> &src)
{
  AccumulateTupleElementsIntoArray<S - 1>(dst, src);
  dst.push_back(Capture(std::get<S - 1>(src)));
}

// -----------------------------------------------------------------------------

template <size_t S, typename... Ts, typename std::enable_if<S == 0, bool>::type>
void telemetry::impl::AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &, std::tuple<Ts...> &&)
{
}

// -----------------------------------------------------------------------------

template <
    size_t S, typename... Ts, typename std::enable_if<(S > 0), bool>::type>
void telemetry::impl::AccumulateTupleElementsIntoArray(
    std::vector<LogValue> &dst, std::tuple<Ts...> &&src)
{
  AccumulateTupleElementsIntoArray<S - 1>(dst, std::move(src));
  dst.push_back(Capture(std::move(std::get<S - 1>(src))));
}

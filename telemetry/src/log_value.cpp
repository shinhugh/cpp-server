#include "log_value.h"

#include <csignal>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------

static void Assert(
    bool);

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateVoid()
{
  return LogValue{};
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateBoolean(
    bool v)
{
  LogValue output;
  output.m_type = Type::BOOLEAN;
  output.m_boolean = v;
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateInteger(
    long long v)
{
  LogValue output;
  output.m_type = Type::INTEGER;
  output.m_integer = v;
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateFloat(
    double v)
{
  LogValue output;
  output.m_type = Type::FLOAT;
  output.m_float = v;
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateString(
    std::string &&v)
{
  LogValue output;
  output.m_type = Type::STRING;
  new (&output.m_string) decltype(output.m_string){std::move(v)};
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateArray(
    std::vector<LogValue> &&v)
{
  LogValue output;
  output.m_type = Type::ARRAY;
  new (&output.m_array) decltype(output.m_array){std::move(v)};
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue telemetry::LogValue::CreateObject(
    std::unordered_map<std::string, LogValue> &&v)
{
  LogValue output;
  output.m_type = Type::OBJECT;
  new (&output.m_object) decltype(output.m_object){std::move(v)};
  return output;
}

// -----------------------------------------------------------------------------

telemetry::LogValue::LogValue()
    : m_type(Type::VOID)
{
}

// -----------------------------------------------------------------------------

telemetry::LogValue::LogValue(
    const LogValue &src)
    : m_type(src.m_type)
{
  switch (m_type)
  {
    case Type::VOID:
      break;
    case Type::BOOLEAN:
      m_boolean = src.m_boolean;
      break;
    case Type::INTEGER:
      m_integer = src.m_integer;
      break;
    case Type::FLOAT:
      m_float = src.m_float;
      break;
    case Type::STRING:
      new (&m_string) decltype(m_string){src.m_string};
      break;
    case Type::ARRAY:
      new (&m_array) decltype(m_array){src.m_array};
      break;
    case Type::OBJECT:
      new (&m_object) decltype(m_object){src.m_object};
      break;
  }
}

// -----------------------------------------------------------------------------

telemetry::LogValue::LogValue(
    LogValue &&src)
    : m_type(src.m_type)
{
  switch (m_type)
  {
    case Type::VOID:
      break;
    case Type::BOOLEAN:
      m_boolean = src.m_boolean;
      break;
    case Type::INTEGER:
      m_integer = src.m_integer;
      break;
    case Type::FLOAT:
      m_float = src.m_float;
      break;
    case Type::STRING:
      new (&m_string) decltype(m_string){std::move(src.m_string)};
      break;
    case Type::ARRAY:
      new (&m_array) decltype(m_array){std::move(src.m_array)};
      break;
    case Type::OBJECT:
      new (&m_object) decltype(m_object){std::move(src.m_object)};
      break;
  }
}

// -----------------------------------------------------------------------------

telemetry::LogValue::~LogValue()
{
  switch (m_type)
  {
    case Type::STRING:
      m_string.std::string::~string();
      break;
    case Type::ARRAY:
      m_array.~vector();
      break;
    case Type::OBJECT:
      m_object.~unordered_map();
      break;
    default:
      break;
  }
}

// -----------------------------------------------------------------------------

telemetry::LogValue &telemetry::LogValue::operator=(
    const LogValue &src)
{
  switch (m_type)
  {
    case Type::STRING:
      m_string.std::string::~string();
      break;
    case Type::ARRAY:
      m_array.~vector();
      break;
    case Type::OBJECT:
      m_object.~unordered_map();
      break;
    default:
      break;
  }

  m_type = src.m_type;
  switch (m_type)
  {
    case Type::VOID:
      break;
    case Type::BOOLEAN:
      m_boolean = src.m_boolean;
      break;
    case Type::INTEGER:
      m_integer = src.m_integer;
      break;
    case Type::FLOAT:
      m_float = src.m_float;
      break;
    case Type::STRING:
      new (&m_string) decltype(m_string){src.m_string};
      break;
    case Type::ARRAY:
      new (&m_array) decltype(m_array){src.m_array};
      break;
    case Type::OBJECT:
      new (&m_object) decltype(m_object){src.m_object};
      break;
  }

  return *this;
}

// -----------------------------------------------------------------------------

telemetry::LogValue &telemetry::LogValue::operator=(
    LogValue &&src)
{
  switch (m_type)
  {
    case Type::STRING:
      m_string.std::string::~string();
      break;
    case Type::ARRAY:
      m_array.~vector();
      break;
    case Type::OBJECT:
      m_object.~unordered_map();
      break;
    default:
      break;
  }

  m_type = src.m_type;
  switch (m_type)
  {
    case Type::VOID:
      break;
    case Type::BOOLEAN:
      m_boolean = src.m_boolean;
      break;
    case Type::INTEGER:
      m_integer = src.m_integer;
      break;
    case Type::FLOAT:
      m_float = src.m_float;
      break;
    case Type::STRING:
      new (&m_string) decltype(m_string){std::move(src.m_string)};
      break;
    case Type::ARRAY:
      new (&m_array) decltype(m_array){std::move(src.m_array)};
      break;
    case Type::OBJECT:
      new (&m_object) decltype(m_object){std::move(src.m_object)};
      break;
  }

  return *this;
}

// -----------------------------------------------------------------------------

telemetry::LogValue::Type telemetry::LogValue::GetType() const
{
  return m_type;
}

// -----------------------------------------------------------------------------

bool telemetry::LogValue::GetBoolean() const
{
  Assert(m_type == Type::BOOLEAN);
  return m_boolean;
}

// -----------------------------------------------------------------------------

long long telemetry::LogValue::GetInteger() const
{
  Assert(m_type == Type::INTEGER);
  return m_integer;
}

// -----------------------------------------------------------------------------

double telemetry::LogValue::GetFloat() const
{
  Assert(m_type == Type::FLOAT);
  return m_float;
}

// -----------------------------------------------------------------------------

const std::string &telemetry::LogValue::GetString() const
{
  Assert(m_type == Type::STRING);
  return m_string;
}

// -----------------------------------------------------------------------------

const std::vector<telemetry::LogValue> &telemetry::LogValue::GetArray() const
{
  Assert(m_type == Type::ARRAY);
  return m_array;
}

// -----------------------------------------------------------------------------

const std::unordered_map<std::string, telemetry::LogValue> &
telemetry::LogValue::GetObject() const
{
  Assert(m_type == Type::OBJECT);
  return m_object;
}

// -----------------------------------------------------------------------------

static void Assert(
    bool condition)
{
  if (!condition)
  {
    std::raise(SIGSEGV);
  }
}

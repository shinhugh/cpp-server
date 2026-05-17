#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------

namespace telemetry
{

// -----------------------------------------------------------------------------

class LogValue
{
public:

  enum class Type
  {
    VOID,
    BOOLEAN,
    INTEGER,
    FLOAT,
    STRING,
    ARRAY,
    OBJECT
  };

public:

  static LogValue CreateVoid();

  static LogValue CreateBoolean(
      bool);

  static LogValue CreateInteger(
      long long);

  static LogValue CreateFloat(
      double);

  static LogValue CreateString(
      std::string &&);

  static LogValue CreateArray(
      std::vector<LogValue> &&);

  static LogValue CreateObject(
      std::unordered_map<std::string, LogValue> &&);

private:

  LogValue();

public:

  LogValue(
      const LogValue &);

  LogValue(
      LogValue &&);

  ~LogValue();

  LogValue &operator=(
      const LogValue &);

  LogValue &operator=(
      LogValue &&);

  Type GetType() const;

  bool GetBoolean() const;

  long long GetInteger() const;

  double GetFloat() const;

  const std::string &GetString() const;

  const std::vector<LogValue> &GetArray() const;

  const std::unordered_map<std::string, LogValue> &GetObject() const;

private:

  Type m_type;

  union
  {
    bool m_boolean;
    long long m_integer;
    double m_float;
    std::string m_string;
    std::vector<LogValue> m_array;
    std::unordered_map<std::string, LogValue> m_object;
  };
};

// -----------------------------------------------------------------------------

}  // namespace telemetry

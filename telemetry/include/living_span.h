#pragma once

#include "span.h"

#include <chrono>
#include <optional>
#include <string>

// -----------------------------------------------------------------------------

namespace telemetry
{

// -----------------------------------------------------------------------------

class LivingSpan : public Span
{
public:

  LivingSpan(
      bool openOnCreate = true);

  LivingSpan(
      const std::string &tag, bool openOnCreate = true);

  LivingSpan(
      std::string &&tag, bool openOnCreate = true);

  LivingSpan(
      const char *tag, bool openOnCreate = true);

  LivingSpan(
      const Span &parentSpan, bool openOnCreate = true);

  LivingSpan(
      const Span &parentSpan, const std::string &tag, bool openOnCreate = true);

  LivingSpan(
      const Span &parentSpan, std::string &&tag, bool openOnCreate = true);

  LivingSpan(
      const Span &parentSpan, const char *tag, bool openOnCreate = true);

  LivingSpan(
      const LivingSpan &) = delete;

  LivingSpan(
      LivingSpan &&);

  ~LivingSpan();

  LivingSpan &operator=(
      const LivingSpan &) = delete;

  LivingSpan &operator=(
      LivingSpan &&);

  void Open();

  void Open(
      std::chrono::steady_clock::time_point);

  void Close();

  void Close(
      std::chrono::steady_clock::time_point);

private:

  bool m_invalid;
  bool m_open;
  std::optional<Span> m_parentSpan;
  std::optional<std::string> m_tag;
};

// -----------------------------------------------------------------------------

}  // namespace telemetry

#pragma once

#include "log_entry.h"
#include "log_sink.h"

#include <optional>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

class StdoutLogSink : public LogSink
{
public:
  virtual void Commit(const LogEntry&) override;
};

// -----------------------------------------------------------------------------

extern std::optional<StdoutLogSink> g_stdoutLogSink;

// -----------------------------------------------------------------------------

} // server

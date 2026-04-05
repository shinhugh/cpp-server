#pragma once

#include "subsystem/subsystem.h"

#include <uv.h>

#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

// -----------------------------------------------------------------------------

namespace server
{

// -----------------------------------------------------------------------------

class UvLoop
{
public:
  UvLoop();
  ~UvLoop();
  bool Run();
  void Quit(bool preemptive = false);
  void ResetQuit();
  bool SetSubsystems(const std::unordered_set<Subsystem*>&);
  bool SetSubsystems(std::unordered_set<Subsystem*>&&);

private:
  void CreateAndStartTimerForSubsystem(Subsystem*);
  void StopTimerForSubsystemAndQueueDestroy(Subsystem*);

private:
  uv_loop_t m_loopHandle;
  bool m_running = false;
  bool m_quit = false;
  std::mutex m_runningQuitMutex;
  std::unordered_set<Subsystem*> m_subsystems;
  std::unordered_map<void*, uv_timer_t> m_subsystemTimerHandles;
};

// -----------------------------------------------------------------------------

extern std::optional<UvLoop> g_uvLoop;

// -----------------------------------------------------------------------------

} // server

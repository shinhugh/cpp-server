#include "uv_loop.h"

#include "subsystem/subsystem.h"

#include <uv.h>

#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// -----------------------------------------------------------------------------

namespace server::impl
{

// -----------------------------------------------------------------------------

struct SubsystemTimerHandleData
{
  SubsystemTimerHandleData(
    std::unordered_map<void*, uv_timer_t>&,
    server::Subsystem&);

  std::unordered_map<void*, uv_timer_t>& m_subsystemTimerHandles;
  server::Subsystem& m_subsystem;
};

// -----------------------------------------------------------------------------

} // server::impl

// -----------------------------------------------------------------------------

std::optional<server::UvLoop> server::g_uvLoop;

// -----------------------------------------------------------------------------

server::impl::SubsystemTimerHandleData::SubsystemTimerHandleData(
  std::unordered_map<void*, uv_timer_t>& subsystemTimerHandles,
  server::Subsystem& subsystem)
  : m_subsystemTimerHandles(subsystemTimerHandles)
  , m_subsystem(subsystem)
{
}

// -----------------------------------------------------------------------------

server::UvLoop::UvLoop()
{
  uv_loop_init(&m_loopHandle);
}

// -----------------------------------------------------------------------------

server::UvLoop::~UvLoop()
{
  uv_loop_close(&m_loopHandle);
}

// -----------------------------------------------------------------------------

bool server::UvLoop::Run()
{
  {
    std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
    if (m_running)
    {
      return false;
    }
    m_running = true;
  }

  for (Subsystem* subsystem : m_subsystems)
  {
    CreateAndStartTimerForSubsystem(subsystem);
  }

  while (true)
  {
    {
      std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
      if (m_quit)
      {
        break;
      }
    }
    uv_run(&m_loopHandle, UV_RUN_ONCE);
  }

  for (Subsystem* subsystem : m_subsystems)
  {
    StopTimerForSubsystemAndQueueDestroy(subsystem);
  }

  while (!m_subsystemTimerHandles.empty())
  {
    uv_run(&m_loopHandle, UV_RUN_ONCE);
  }

  {
    std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
    m_quit = false;
    m_running = false;
  }

  return true;
}

// -----------------------------------------------------------------------------

void server::UvLoop::Quit(bool preemptive)
{
  std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
  if (!preemptive && !m_running)
  {
    return;
  }
  m_quit = true;
}

// -----------------------------------------------------------------------------

void server::UvLoop::ResetQuit()
{
  std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
  m_quit = false;
}

// -----------------------------------------------------------------------------

bool server::UvLoop::SetSubsystems(
  const std::unordered_set<Subsystem*>& subsystems)
{
  std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
  if (m_running)
  {
    return false;
  }
  m_subsystems = subsystems;
  return true;
}

// -----------------------------------------------------------------------------

bool server::UvLoop::SetSubsystems(std::unordered_set<Subsystem*>&& subsystems)
{
  std::lock_guard<std::mutex> lock{ m_runningQuitMutex };
  if (m_running)
  {
    return false;
  }
  m_subsystems = std::move(subsystems);
  return true;
}

// -----------------------------------------------------------------------------

void server::UvLoop::CreateAndStartTimerForSubsystem(Subsystem* subsystem)
{
  uv_timer_t& timerHandle = m_subsystemTimerHandles[subsystem];
  uv_timer_init(&m_loopHandle, &timerHandle);
  uv_handle_set_data(
    reinterpret_cast<uv_handle_t*>(&timerHandle),
    new impl::SubsystemTimerHandleData{ m_subsystemTimerHandles, *subsystem });
  uv_timer_start(
    &timerHandle,
    [](uv_timer_t* timerHandle)
    {
      uv_handle_t* baseHandle = reinterpret_cast<uv_handle_t*>(timerHandle);
      void* handleData = uv_handle_get_data(baseHandle);
      Subsystem& subsystem
        = static_cast<impl::SubsystemTimerHandleData*>(handleData)
        ->m_subsystem;
      subsystem.Process();
    },
    0,
    100);
}

// -----------------------------------------------------------------------------

void server::UvLoop::StopTimerForSubsystemAndQueueDestroy(Subsystem* subsystem)
{
  uv_timer_t& timerHandle = m_subsystemTimerHandles[subsystem];
  uv_timer_stop(&timerHandle);
  uv_close(
    reinterpret_cast<uv_handle_t*>(&timerHandle),
    [](uv_handle_t* handle)
    {
      impl::SubsystemTimerHandleData* handleData
        = static_cast<impl::SubsystemTimerHandleData*>(
          uv_handle_get_data(handle));
      handleData->m_subsystemTimerHandles.erase(&handleData->m_subsystem);
      delete handleData;
    });
}

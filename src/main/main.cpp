#include "uv_loop/uv_loop.h"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <optional>
#include <thread>

// -----------------------------------------------------------------------------

static void NotifyQuit();
static void MonitorForQuit();

// -----------------------------------------------------------------------------

// TODO: If there is a guarantee that the OS will never interrupt the quit
//       monitor thread with a signal, the following changes should be made:
//       - change s_quitMutex from recursive_mutex to mutex
//       - change s_quitCv from condition_variable_any to condition_variable
//       - replace s_quitCv.wait_for() with s_quitCv.wait()
//       - remove #include <chrono>
static bool s_quit;
static std::recursive_mutex s_quitMutex;
static std::condition_variable_any s_quitCv;

// -----------------------------------------------------------------------------

int main()
{
  std::signal(SIGINT, [](int) { NotifyQuit(); });
  std::signal(SIGTERM, [](int) { NotifyQuit(); });

  server::g_uvLoop.emplace();

  std::thread monitorForQuitThread{ MonitorForQuit };

  server::g_uvLoop->Run();

  NotifyQuit();
  monitorForQuitThread.join();
  server::g_uvLoop->ResetQuit();

  return 0;
}

// -----------------------------------------------------------------------------

static void NotifyQuit()
{
  {
    std::lock_guard lock{ s_quitMutex };
    s_quit = true;
  }
  s_quitCv.notify_one();
}

// -----------------------------------------------------------------------------

static void MonitorForQuit()
{
  {
    std::unique_lock lock{ s_quitMutex };
    while (!s_quit)
    {
      s_quitCv.wait_for(lock, std::chrono::milliseconds(500));
    }
  }

  server::g_uvLoop->Quit(true);
}

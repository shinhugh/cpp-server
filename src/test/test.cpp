#include "async/async_manager.h"
#include "async/async.h"
#include "async/future.h"
#include "telemetry/log_manager.h"
#include "telemetry/log.h"
#include "telemetry/stdout_log_sink.h"

#include <chrono>
#include <string>
#include <thread>

int main()
{
  server::InitializeProcessSpan();

  server::g_stdoutLogSink.emplace();
  server::g_logManager.emplace();
  server::g_logManager->RegisterLogSink(*server::g_stdoutLogSink);

  server::g_asyncManager.emplace();

  server::Future<int> coroutineFuture = server::RunTaskOnNewCoroutine<int>([]()
    {
      LOG_DEBUG("  OUTER COROUTINE 1:   start");

      server::RunTaskOnNewCoroutine<void>([]()
        {
          LOG_DEBUG("    INNER COROUTINE 1: start");
          LOG_DEBUG("    INNER COROUTINE 1: finish");
        }).Await();

      LOG_DEBUG("  OUTER COROUTINE 1:   checkpoint");

      server::RunTaskOnNewCoroutine<void>([]()
        {
          LOG_DEBUG("    INNER COROUTINE 2: start");
          LOG_DEBUG("    INNER COROUTINE 2: finish");
        }).Await();

      LOG_DEBUG("  OUTER COROUTINE 1:   finish");

      return 1;
    });

  server::Future<int> threadFuture = server::RunTaskOnNewThread<int>([]()
    {
      LOG_DEBUG("  OUTER THREAD 1:      hello");

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      server::RunTaskOnNewThread<void>([]()
        {
          LOG_DEBUG("    INNER THREAD 1:    hello");

          std::this_thread::sleep_for(std::chrono::milliseconds(500));

          LOG_DEBUG("    INNER THREAD 1:    goodbye");
        }).Await();

      LOG_DEBUG("  OUTER THREAD 1:      goodbye");

      return 2;
    });

  LOG_DEBUG("MAIN:                  iteration 0");
  server::g_asyncManager->Process();
  for (int i = 0; i < 9; i++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::string log{ "MAIN:                  iteration " };
    log.append(std::to_string(i + 1));
    LOG_DEBUG(log);
    server::g_asyncManager->Process();
  }

  int result = coroutineFuture.Await();
  std::string log{ "MAIN:                  coroutine result: " };
  log.append(std::to_string(result));
  LOG_DEBUG(log);
  result = threadFuture.Await();
  log = "MAIN:                  thread result:    ";
  log.append(std::to_string(result));
  LOG_DEBUG(log);

  server::TerminateProcessSpan();

  return 0;
}

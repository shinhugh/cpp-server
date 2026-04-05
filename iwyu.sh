#!/bin/bash

# Must be run from project's root directory

include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/async/async_manager.cpp
include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/async/async.cpp
include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/async/future_state.cpp
include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/async/future.cpp
include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/async/thread_local_state.cpp

include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/main/main.cpp

include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/subsystem/subsystem.cpp

include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/telemetry/living_span.cpp

include-what-you-use -Xiwyu --no_fwd_decls -Xiwyu --quoted_includes_first -Isrc -D PLATFORM_POSIX src/uv_loop/uv_loop.cpp

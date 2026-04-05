#!/bin/bash

# Must be run from project's root directory

rm -rf build

mkdir -p build/async
clang++ src/async/thread_local_state.cpp -c -o build/async/thread_local_state.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/main
clang++ src/main/main.cpp -c -o build/main/main.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/subsystem
clang++ src/subsystem/subsystem.cpp -c -o build/subsystem/subsystem.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/telemetry
clang++ src/telemetry/living_span.cpp -c -o build/telemetry/living_span.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/test
clang++ src/test/test.cpp -c -o build/test/test.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/uv_loop
clang++ src/uv_loop/uv_loop.cpp -c -o build/uv_loop/uv_loop.o -Isrc -D PLATFORM_POSIX -g -Wall -Wextra

clang++               \
  build/async/*.o     \
  build/main/*.o      \
  build/subsystem/*.o \
  build/telemetry/*.o \
  build/uv_loop/*.o   \
  -luv                \
  -o build/main.out

clang++               \
  build/async/*.o     \
  build/subsystem/*.o \
  build/telemetry/*.o \
  build/test/*.o      \
  build/uv_loop/*.o   \
  -luv                \
  -o build/test.out

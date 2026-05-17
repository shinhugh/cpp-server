#!/bin/bash

# Must be run from project's root directory

mkdir -p build/_include/async
cp async/include/* build/_include/async/

mkdir -p build/_include/telemetry
cp telemetry/include/* build/_include/telemetry/

mkdir -p build/async
clang++ async/src/async.cpp             -c -o build/async/async.o             -Ibuild/_include -Ibuild/_include/async -D PLATFORM_POSIX -g -Wall -Wextra
clang++ async/src/coroutine_context.cpp -c -o build/async/coroutine_context.o -Ibuild/_include -Ibuild/_include/async -D PLATFORM_POSIX -g -Wall -Wextra
clang++ async/src/future.cpp            -c -o build/async/future.o            -Ibuild/_include -Ibuild/_include/async -D PLATFORM_POSIX -g -Wall -Wextra
clang++ async/src/promise.cpp           -c -o build/async/promise.o           -Ibuild/_include -Ibuild/_include/async -D PLATFORM_POSIX -g -Wall -Wextra
llvm-ar rcs build/libasync.a build/async/*.o

mkdir -p build/telemetry
clang++ telemetry/src/living_span.cpp -c -o build/telemetry/living_span.o -Ibuild/_include -Ibuild/_include/telemetry -D PLATFORM_POSIX -g -Wall -Wextra
clang++ telemetry/src/log.cpp         -c -o build/telemetry/log.o         -Ibuild/_include -Ibuild/_include/telemetry -D PLATFORM_POSIX -g -Wall -Wextra
clang++ telemetry/src/log_value.cpp   -c -o build/telemetry/log_value.o   -Ibuild/_include -Ibuild/_include/telemetry -D PLATFORM_POSIX -g -Wall -Wextra
clang++ telemetry/src/trace.cpp       -c -o build/telemetry/trace.o       -Ibuild/_include -Ibuild/_include/telemetry -D PLATFORM_POSIX -g -Wall -Wextra
llvm-ar rcs build/libtelemetry.a build/telemetry/*.o

mkdir -p build/main
clang++ main/src/main.cpp -c -o build/main/main.o -Ibuild/_include -Ibuild/_include/main -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/test
clang++ test/src/test.cpp -c -o build/test/test.o -Ibuild/_include -Ibuild/_include/test -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build
clang++           \
  build/main/*.o  \
  -Lbuild         \
  -lasync         \
  -ltelemetry     \
  -lboost_context \
  -o build/main.out

mkdir -p build
clang++           \
  build/test/*.o  \
  -Lbuild         \
  -lasync         \
  -ltelemetry     \
  -lboost_context \
  -o build/test.out

#!/bin/bash

# Must be run from project's root directory

mkdir -p build/main
clang++ main/src/main.cpp -c -o build/main/main.o -Ibuild/_include -Ibuild/_include/main -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build/test
clang++ test/src/test.cpp -c -o build/test/test.o -Ibuild/_include -Ibuild/_include/test -D PLATFORM_POSIX -g -Wall -Wextra

mkdir -p build
clang++          \
  build/main/*.o \
  -Lbuild        \
  -o build/main.out

mkdir -p build
clang++          \
  build/test/*.o \
  -Lbuild        \
  -o build/test.out

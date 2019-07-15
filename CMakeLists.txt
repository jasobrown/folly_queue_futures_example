cmake_minimum_required(VERSION 3.12)
project(jeb-threading)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER "/usr/bin/clang-7")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-7")
set(CMAKE_AR "/usr/bin/llvm-ar-7")
set(CMAKE_LINKER "/usr/bin/lld-7")
set(CMAKE_NM "/usr/bin/llvm-nm-7")
set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump-7")
set(CMAKE_RANLIB "/usr/bin/llvm-ranlib")

add_executable(jeb-threading
  src/main.cpp
  src/pmem_event.cpp
)

target_include_directories(jeb-threading
  PUBLIC
  include
  "/opt/dev/cpp/folly/build/default/opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/folly/include"
#  "/opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/boost/include"
)

# linking the .so links to a shared library, which is already compiled &
# dynamically locates the deps at runtime
target_link_libraries(jeb-threading
  pthread
  # would prefer to use my own boost rather than the system, but ...
#  /opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/boost/lib/libboost_context.a
#  /opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/boost/lib/libboost_thread.a
#  /opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/boost/lib/libboost_system.a
  /opt/dev/cpp/folly/build/default/opt/dev/cockroach/19.1/native/x86_64-pc-linux-gnu/folly/lib/libfolly.a
  glog
  /usr/lib/x86_64-linux-gnu/libiberty.a
  /usr/lib/x86_64-linux-gnu/libdouble-conversion.a
  gflags
  dl
  atomic
  boost_context
  boost_thread
  boost_system
  event
  hwloc
)

# linking the .a is trying to pull in a static library
# which needs all of it's dependencies pulled in at link time (I think)
# target_link_libraries(pmemart
#   /opt/local/lib/libpmem.a
#   /opt/local/lib/libpmemobj.a
#   pthread
#   ndctl
#   daxctl
#   dl
# )

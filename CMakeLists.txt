cmake_minimum_required(VERSION 3.5)
project(UDPForwarder)
set(CMAKE_CXX_STANDARD 17)

find_package(rlib)
find_package(Threads)

set(CMAKE_CXX_FLAGS_DEBUG "-g -DMALLOC_CHECK_=2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-user-defined-literals")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix -Wno-terminate")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

string(TIMESTAMP _COMPILE_TIME %Y-%m-%dT%H:%M:%SZ UTC)
add_definitions(-DCOMPILE_TIME=${_COMPILE_TIME})

set(CMAKE_VERBOSE_MAKEFILE ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# MacOS has no MSG_NOSIGNAL define? I'm not sure if this will lead to EPIPE.
if(APPLE)
    add_definitions(-DMSG_NOSIGNAL=0)
endif()

include_directories(./src)

# TODO
set(SRC src/main.cc)

add_executable(udp-forwarder ${SRC})
target_link_libraries(udp-forwarder r)
target_link_libraries(udp-forwarder Threads::Threads)

# target_link_libraries(udp-forwarder -static-libgcc -static-libstdc++)
#if(FOR_M32)
#    set_target_properties(... PROPERTIES COMPILE_FLAGS -m32 LINK_FLAGS -m32 )
#endif(FOR_M32)


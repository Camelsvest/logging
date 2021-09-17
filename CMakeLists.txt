cmake_minimum_required(VERSION 2.8.12)
project(logging)

option(DISABLE_TEST "enable to compile test program" OFF)

set(CMAKE_C_FLAGS "-Wall")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
  add_definitions("-DDBG_ENABLED")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

set(LOGGING_SRCS)
list(APPEND LOGGING_SRCS bipbuffer.c)
list(APPEND LOGGING_SRCS string_buf.c string_buf.h)
list(APPEND LOGGING_SRCS logging.c logging.h)

add_library(logging STATIC ${LOGGING_SRCS})

if (NOT DISABLE_TEST)
    add_subdirectory(test)
endif()

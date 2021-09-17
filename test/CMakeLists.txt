cmake_minimum_required(VERSION 2.8.12)
project(logging_test)

set(CMAKE_C_FLAGS "-Wall -O0")
add_definitions("-DDBG_ENABLED")

set(SRCS)
list(APPEND SRCS main.c)

include_directories(..)
link_directories(..)

add_executable(logging_test ${SRCS})

target_link_libraries(logging_test logging pthread)


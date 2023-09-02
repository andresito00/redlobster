cmake_minimum_required(VERSION 3.18.2)
project(simple-cross-exe)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

include_directories(order_book/)
include_directories(util/)

set(simple_cross_src main.cpp simple_cross.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG") # need symbols for gdb
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wbinding-in-condition")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicate-enum")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# TODO: -Wlifetime

add_subdirectory(order_book)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_CXX_CLANG_TIDY clang-tidy -fix -checks=google-*,clang-analyzer-*,-header-filter=.*)

add_executable(simple_cross ${simple_cross_src})

target_link_libraries(
  simple_cross
  order_book
)

# testing binaries
add_library(test_utils test/test_utils.cpp)

add_executable(test_kill_to_empty test/test_kill_to_empty.cpp )
target_link_libraries(test_kill_to_empty PRIVATE order_book test_utils)

enable_testing()
set(env CTEST_OUTPUT_ON_FAILURE 1)

add_test(
  NAME test_kill_to_empty
  COMMAND $<TARGET_FILE:test_kill_to_empty>
  )

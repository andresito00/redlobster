cmake_minimum_required(VERSION 3.18.2)
project(simple-cross-exe)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG") # need symbols for gdb
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
if(CMAKE_CXX_COMPILER STREQUAL "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wbinding-in-condition")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicate-enum")
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
set(CMAKE_CXX_CLANG_TIDY clang-tidy -fix -checks=google-*,clang-analyzer-*,-header-filter=.*)

include_directories(order_book/)
include_directories(util/)
add_subdirectory(order_book)
add_library(sc simple_cross.cpp)
add_executable(simple_cross main.cpp)

target_link_libraries(
  simple_cross
  sc
  order
)

# testing binaries
add_library(test_utils test/test_utils.cpp)

add_executable(test_kill_to_empty test/test_kill_to_empty.cpp )
target_link_libraries(test_kill_to_empty PRIVATE order test_utils)

add_executable(test_partial_fill test/test_partial_fill.cpp )
target_link_libraries(test_partial_fill PRIVATE order test_utils)

add_executable(test_full_fills_only test/test_full_fills_only.cpp )
target_link_libraries(test_full_fills_only PRIVATE order test_utils)

enable_testing()
add_test(NAME tests
  COMMAND "${CMAKE_CURRENT_LIST_DIR}/test.sh"
  WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

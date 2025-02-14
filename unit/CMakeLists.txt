cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(runnable_lib SHARED ../src/runnable.cpp)
target_include_directories(runnable_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(tcp_lib ../src/tcp.cpp)
target_include_directories(tcp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tcp_lib PRIVATE runnable_lib)

add_library(data_manager_lib INTERFACE)
target_include_directories(data_manager_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include/dataManager.hpp)
target_link_libraries(tcp_lib PRIVATE runnable_lib)

add_library(queue_lib INTERFACE)
target_include_directories(queue_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include/queue.hpp)
target_link_libraries(tcp_lib PRIVATE runnable_lib)

add_library(tp_lib SHARED ../src/threadPool.cpp)
target_include_directories(tp_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
target_link_libraries(tcp_lib PRIVATE runnable_lib)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  test_tcp_server
  test_tcp_server.cpp
  )

add_executable(
  test_data_manager
  test_data_manager.cpp
  )

add_executable(
  test_queue
  test_queue.cpp
  )

add_executable(
  test_thread_pool
  test_thread_pool.cpp
  )

target_link_libraries( test_tcp_server gtest gtest_main tcp_lib)
target_link_libraries( test_data_manager gtest gtest_main data_manager_lib)
target_link_libraries( test_queue gtest gtest_main queue_lib)
target_link_libraries( test_thread_pool gtest gtest_main tp_lib)

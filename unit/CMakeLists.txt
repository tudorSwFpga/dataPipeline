cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(spdlog REQUIRED)


add_library(runnable_lib SHARED ../src/runnable.cpp)
target_include_directories(runnable_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(tcp_lib ../src/tcp.cpp)
target_include_directories(tcp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tcp_lib PRIVATE runnable_lib spdlog::spdlog data_manager_lib)

#template library
add_library(data_manager_lib INTERFACE)
target_include_directories(data_manager_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(data_manager_lib INTERFACE  runnable_lib spdlog::spdlog queue_lib)

#template library
add_library(queue_lib INTERFACE)
target_include_directories(queue_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(queue_lib INTERFACE runnable_lib spdlog::spdlog)

add_library(tp_lib ../src/threadPool.cpp)
target_include_directories(tp_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(tp_lib PRIVATE spdlog::spdlog)

add_library(udp_lib ../src/udp.cpp)
target_include_directories(udp_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(udp_lib PRIVATE spdlog::spdlog runnable_lib data_manager_lib)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0  # Use the latest stable version
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
  test_udp
  test_udp.cpp
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

target_link_libraries( test_tcp_server GTest::gtest_main tcp_lib data_manager_lib queue_lib)
target_link_libraries( test_udp GTest::gtest_main udp_lib data_manager_lib queue_lib)
target_link_libraries( test_data_manager GTest::gtest_main data_manager_lib)
target_link_libraries( test_queue GTest::gtest_main queue_lib)
target_link_libraries( test_thread_pool GTest::gtest_main tp_lib)

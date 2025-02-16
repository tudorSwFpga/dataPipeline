project ("testFramework")
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#TODO: at a certain point, build using installed lib
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(STATUS "spdlog not found, installing through apt")
    execute_process(COMMAND sudo apt install -y libspdlog-dev)
endif()

find_package(Boost QUIET COMPONENTS property_tree)
if (NOT Boost_PROPERTY_TREE_FOUND)
    message(STATUS "Boost.PropertyTree not found, installing via apt...")
    execute_process(COMMAND sudo apt install -y libboost-dev)
endif()



add_executable(testFramework test.cpp)
target_sources(testFramework
    PUBLIC
        "../src/runnable.cpp"
        "../src/dataFramework.cpp"
        "../src/proxy.cpp"
        "../src/tcp.cpp"
        "../src/udp.cpp"
        "../src/threadPool.cpp"
        "../include/consumer.hpp"
    )

target_include_directories(testFramework PUBLIC "../include")
target_link_libraries(testFramework spdlog::spdlog)

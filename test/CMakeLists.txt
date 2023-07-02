project ("testFramework")
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")


#TODO: at a certain point, build using installed lib
find_package(Threads REQUIRED)
add_subdirectory("../plog" "plog")
add_executable(testFramework test.cpp)
target_sources(testFramework 
    PUBLIC 
        "../src/runnable.cpp"
        "../src/dataFramework.cpp"
        "../src/proxy.cpp"
        "../src/tcp.cpp"
        "../src/threadPool.cpp"
        "../include/consumer.hpp"
    )

target_include_directories(testFramework PUBLIC "../include")
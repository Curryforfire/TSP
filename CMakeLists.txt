cmake_minimum_required(VERSION 4.2)
project(TSP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(TSP
        main.cpp
        src/Instance.cpp
        src/Tour.cpp
        src/NeighborLists.cpp
        src/Construct.cpp
        src/LinKernighan.cpp
)

target_include_directories(TSP PRIVATE include)

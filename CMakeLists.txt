cmake_minimum_required(VERSION 3.18)

project(seabang)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Grab the c++ compiler that cmake found and pass into seabang so it uses the correct one for the platform it is run on.
add_definitions(-DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")

add_executable(seabang source/seabang.cpp source/dependencies.cpp source/TinyTools.cpp)
target_link_libraries(seabang stdc++ pthread)

install(TARGETS seabang)

message(${CMAKE_CXX_COMPILER})
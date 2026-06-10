cmake_minimum_required(VERSION 3.21)
project(ev_cockpit_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

enable_testing()

add_subdirectory(bms)
add_subdirectory(ota)
add_subdirectory(app)

cmake_minimum_required(VERSION 3.21)
project(EXCHANGE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.40 REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(Server Server.cpp Common.hpp)

target_link_libraries(Server PRIVATE Boost::system nlohmann_json::nlohmann_json)

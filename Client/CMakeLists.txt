cmake_minimum_required(VERSION 3.21)
project(EXCHANGE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost 1.40 REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

add_executable(Client Client.cpp Common.hpp)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(Client PRIVATE Boost::system nlohmann_json::nlohmann_json)

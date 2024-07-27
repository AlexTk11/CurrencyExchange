cmake_minimum_required(VERSION 3.21)
project(NTProgress)


find_package(Boost 1.40 REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})


set(SERVER_SOURCES Server.cpp)

add_library(ServerLib ${SERVER_SOURCES})

add_executable(Server Server.cpp Common.hpp)

target_link_libraries(Server PRIVATE Boost::system nlohmann_json::nlohmann_json)

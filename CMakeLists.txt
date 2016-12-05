cmake_minimum_required(VERSION 2.8)
project(discrete_sims)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp utils.cpp)

add_executable(dump_sim ${SOURCE_FILES})

target_link_libraries(dump_sim simlib)
target_link_libraries(dump_sim m)

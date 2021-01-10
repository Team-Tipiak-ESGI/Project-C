# cmake_minimum_required(VERSION <specify CMake version here>)
project(Project_C C)

set(CMAKE_C_STANDARD 11)
cmake_minimum_required(VERSION 3.16)

add_executable(TUCS_client src/client.c src/PacketTypes.h)
add_executable(TUCS_server src/server.c src/PacketTypes.h)

find_package(OpenSSL REQUIRED)
target_link_libraries(TUCS_server OpenSSL::SSL)
target_link_libraries(TUCS_client OpenSSL::SSL)
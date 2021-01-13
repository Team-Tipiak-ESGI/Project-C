project(Project_C C)

set(CMAKE_C_STANDARD 11)
cmake_minimum_required(VERSION 3.16)

add_executable(TUCS_client src/client/Client.c src/shared/PacketTypes.h src/client/Connection.c src/client/Connection.h src/client/SendFile.h src/client/SendFile.c)
add_executable(TUCS_server src/server/Main.c src/shared/PacketTypes.h src/server/Connection.c src/server/Connection.h src/server/Server.c src/server/Server.h src/server/ServerConfiguration.h src/shared/hash.c src/shared/hash.h src/server/Client.h)

target_compile_options(TUCS_client PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(TUCS_server PRIVATE -Wall -Wextra -pedantic -Werror)

find_package(OpenSSL REQUIRED)
target_link_libraries(TUCS_server OpenSSL::SSL)
target_link_libraries(TUCS_client OpenSSL::SSL)
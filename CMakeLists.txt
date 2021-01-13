project(Project_C C)

set(CMAKE_C_STANDARD 11)
cmake_minimum_required(VERSION 3.16)

add_executable(TUCS_client src/client/client.c src/shared/PacketTypes.h src/client/connection.c src/client/connection.h src/client/sendFile.h src/client/sendFile.c)
add_executable(TUCS_server src/server/server.c src/shared/PacketTypes.h src/server/connection.c src/server/connection.h src/server/serve.c src/server/serve.h src/server/configuration.h src/shared/hash.c src/shared/hash.h)

target_compile_options(TUCS_client PRIVATE -Wall -Wextra -pedantic -Werror)
target_compile_options(TUCS_server PRIVATE -Wall -Wextra -pedantic -Werror)

find_package(OpenSSL REQUIRED)
target_link_libraries(TUCS_server OpenSSL::SSL)
target_link_libraries(TUCS_client OpenSSL::SSL)
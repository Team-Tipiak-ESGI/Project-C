project(Project_C C)

set(CMAKE_C_STANDARD 11)
cmake_minimum_required(VERSION 3.16)

add_executable(TUCS_client src/client/Client.c
        src/shared/PacketTypes.h src/client/Connection.c src/client/Connection.h src/client/SendFile.h src/client/SendFile.c src/shared/ChunkSize.h
        src/shared/Configuration.c src/shared/Configuration.h src/shared/Item.c src/shared/Item.h)

add_executable(TUCS_server src/server/Main.c
        # Shared
        src/shared/PacketTypes.h src/shared/ChunkSize.h

        src/server/Connection.c src/server/Connection.h
        src/server/Server.c src/server/Server.h

        # Headers
        src/server/ServerConfiguration.h
        src/server/Client.h

        # Database
        src/server/Database.c src/server/Database.h
        src/server/MongoConnection.h
        src/shared/Configuration.c src/shared/Configuration.h src/shared/Item.c src/shared/Item.h)

target_compile_options(TUCS_client PRIVATE -Wall -Wextra)
#target_compile_options(TUCS_server PRIVATE -Wall -Wextra)

find_package(OpenSSL REQUIRED)
target_link_libraries(TUCS_server OpenSSL::SSL)
target_link_libraries(TUCS_client OpenSSL::SSL)

include_directories("/usr/include/libmongoc-1.0")
include_directories("/usr/include/libbson-1.0")

target_link_libraries(TUCS_server /usr/lib/x86_64-linux-gnu/libmongoc-1.0.so.0 /usr/lib/x86_64-linux-gnu/libbson-1.0.so.0)


file(COPY resources DESTINATION .)
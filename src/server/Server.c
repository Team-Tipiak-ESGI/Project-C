#include <stdio.h>
#include <stdlib.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <mongoc/mongoc.h>

#include "../shared/PacketTypes.h"
#include "../shared/ChunkSize.h"
#include "ServerConfiguration.h"
#include "Client.h"

#include "MongoConnection.h"
#include "Database.h"
#include "Requests.h"

/**
 * Serve the connection
 * @param ssl
 */
void servlet(SSL *ssl, ServerConfiguration server, MongoConnection* mongoConnection) {
    char buffer[CHUNK_SIZE];
    int bytes;

    printf("Serve %p\n", ssl);

    int accept = SSL_accept(ssl);

    printf("Accept %d\n", accept);

    if (accept < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        Client client;

        client.chunkSent = 0;

        while (1) {
            bytes = SSL_read(ssl, buffer, CHUNK_SIZE); /* get request */

            if (bytes == 0) break;

            const unsigned char firstByte = buffer[0];
            const char * content = buffer + 1;

            printf("Message from socket (%d) (packet type: %d) : %s\n", bytes, firstByte, content);

            switch (firstByte) {
                case USERNAME:
                    Username(&client, &server, ssl, mongoConnection, content);
                    break;

                case PASSWORD:
                    Password(&client, &server, ssl, mongoConnection, content);
                    break;

                case CREATE_USER:
                    CreateUser(&client, &server, ssl, mongoConnection, content);
                    break;

                case CREATE_FILE:
                    CreateFile(&client, &server, ssl, mongoConnection, content);
                    break;

                case FILE_SIZE:
                    FileSize(&client, &server, ssl, mongoConnection, content);
                    break;

                case FILE_CONTENT:
                    FileContent(&client, &server, ssl, mongoConnection, content);
                    break;

                case READ_FILE:
                    ReadFile(&client, &server, ssl, mongoConnection, content);
                    break;

                case DELETE_FILE:
                    DeleteFile(&client, &server, ssl, mongoConnection, content);
                    break;

                case LIST_FILES:
                    ListFiles(&client, &server, ssl, mongoConnection, content);
                    break;

                // TODO: Add file sharing over HTTPS
                default:
                    // HTTPS response
                    SSL_write(ssl, "HTTP/1.1 405 Method Not Allowed\n"
                                   "Content-Length: 33\n"
                                   "Content-Type: text/html;\n"
                                   "\n"
                                   "405 This is not a http(s) server!", CHUNK_SIZE);
                    break;
            }
        }

        free(client.password);
        free(client.username);
    }
}

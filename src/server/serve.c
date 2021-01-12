#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../shared/PacketTypes.h"
#define CHUNK_SIZE 1024

/**
 * Verify if the given credentials are valid
 * @param username
 * @param password
 * @return 1 if valid, 0 if not
 */
unsigned char verifyUser(char* username, char* password) {
    if (username == NULL && password == NULL) {
        return 0;
    }

    // TODO: Verify using values in database
    const char* valid_username = "quozul\0";
    const char* valid_password = "password\0";

    return (strcmp(username, valid_username) == 0 && strcmp(password, valid_password) == 0);
}

/**
 * Serve the connection
 * @param ssl
 */
void servlet(SSL *ssl) {
    char buffer[CHUNK_SIZE];
    int sd, bytes;

    if (SSL_accept(ssl) < 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        char* username;
        char* password;

        while (1) {
            bytes = SSL_read(ssl, buffer, 1024); /* get request */

            if (bytes == 0) break;

            const char firstByte = buffer[0];
            char* content = buffer + 1;

            printf("Message from socket (%d) (packet type: %d) : %s\n", bytes, firstByte, content);

            switch (firstByte) {
                case USERNAME:
                    // Copy username to variable
                    username = malloc(sizeof(char) * strlen(content));
                    strncpy(username, content, strlen(content));
                    break;
                case PASSWORD:
                    // Copy password to variable
                    password = malloc(sizeof(char) * strlen(content));
                    strncpy(password, content, strlen(content));
                    break;
                case FILE_CONTENT:
                    if (verifyUser(username, password)) {
                        printf("User %s sent file data\n", username);
                    } else {
                        printf("User is not logged in!\n");
                    }
                    break;
                default:
                    // HTTPS response
                    SSL_write(ssl, "HTTP/1.1 405 Method Not Allowed\n"
                                   "Content-Length: 33\n"
                                   "Content-Type: text/html;\n"
                                   "\n"
                                   "405 This is not a http(s) server!", 1024);
                    break;
            }
        }
    }

    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */

    printf("Connection closed.\n");
}
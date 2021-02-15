#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (linux) /* Linux */

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */

    #include <openssl/crypto.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>

#endif

#define PORT 8080
#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "ServerConfiguration.h"
#include "Connection.h"
#include "Server.h"

#include "Database.h"

int main(void) {
    ServerConfiguration serverConfiguration;
    serverConfiguration.port = 8080;
    serverConfiguration.rootDir = "/home/erwan/tucs_server/";

    SSL_CTX *ctx;
    int server;

    // TODO: Put cert files in config file
    char currentDirectory[1024] = {0};
    getcwd(currentDirectory, 1024);

    char certFile[1024] = {0};
    strcpy(certFile, currentDirectory);
    strcat(certFile, "/resources/server.cert");

    char keyFile[1024] = {0};
    strcpy(keyFile, currentDirectory);
    strcat(keyFile, "/resources/server.key");

    SSL_library_init();

    ctx = initServerCTX();        /* initialize SSL */
    loadCertificates(ctx, certFile, keyFile); /* load certs */

    // TODO: Put port in config file
    server = openListener(PORT);    /* create server socket */

    // Open database
    MongoConnection* mongoConnection = MongoConnection__init();
    MongoConnection__createUser(mongoConnection, "quozul", "test2");
    MongoConnection__getUser(mongoConnection, "quozul", "test2");

    // TODO: Create default table if not exists

    printf("Server listening...\n");

    // Main loop
    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;

        int client = accept(server, (struct sockaddr *) &addr, &len);  /* accept connection as usual */
        // TODO: Log connections to a file
        printf("Connection from: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */

        int pid = fork();

        if (pid == 0) {
            // If in fork
            servlet(ssl, serverConfiguration, mongoConnection);         /* service connection */
        }
    }

    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <pwd.h>

#include <openssl/err.h>

#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "ServerConfiguration.h"
#include "Connection.h"
#include "Server.h"
#include "../shared/Configuration.h"

#include "Database.h"

int main(void) {
    struct passwd *pw = getpwuid(getuid());
    char *tucsDir = pw->pw_dir;
    strcat(tucsDir, "/.tucs");

    struct stat st = {0};
    if (stat(tucsDir, &st) == -1) {
        mkdir(tucsDir, 0700);
        printf("Creating ~/.tucs directory. Full path: [%s]", tucsDir);
    }

    char *confPath = malloc(strlen(tucsDir) + 13);
    strcpy(confPath, tucsDir);
    strcat(confPath, "/");
    strcat(confPath, "tucs.conf");

    Item * config = Configuration__loadFromFile(confPath);

    ServerConfiguration serverConfiguration;
    serverConfiguration.port = atoi(Item__getByKey(config, "port")->value);

    char * rootDir = Item__getByKey(config, "rootDir")->value;
    serverConfiguration.rootDir = malloc(strlen(tucsDir) + strlen(rootDir) + 3);
    strcpy(serverConfiguration.rootDir, tucsDir);
    strcat(serverConfiguration.rootDir, "/");
    strcat(serverConfiguration.rootDir, rootDir);
    strcat(serverConfiguration.rootDir, "/");

    // Create directory if not exists
    if (stat(serverConfiguration.rootDir, &st) == -1) {
        mkdir(serverConfiguration.rootDir, 0700);
    }

    SSL_CTX *ctx;
    int server;

    char certFile[1024] = {0};
    strcpy(certFile, tucsDir);
    strcat(certFile, "/");
    strcat(certFile, Item__getByKey(config, "certFile")->value);

    char keyFile[1024] = {0};
    strcpy(keyFile, tucsDir);
    strcat(keyFile, "/");
    strcat(keyFile, Item__getByKey(config, "keyFile")->value);

    SSL_library_init();

    ctx = initServerCTX();        /* initialize SSL */
    loadCertificates(ctx, certFile, keyFile); /* load certs */

    server = openListener(serverConfiguration.port);    /* create server socket */

    // Open database
    MongoConnection* mongoConnection = MongoConnection__init(Item__getByKey(config, "dbURI")->value);

    // TODO: Create default table if not exists

    printf("Server listening...\n");

    // Main loop
    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);

        int client = accept(server, (struct sockaddr *) &addr, &len);  // Accept connection as usual
        // TODO: Log connections to a file
        printf("Connection from: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        int pid = fork();

        if (pid == 0) {
            // If in fork
            printf("Forking\n");

            SSL * ssl = SSL_new(ctx); // Get new SSL state with context
            SSL_set_fd(ssl, client); // set connection socket to SSL state

            servlet(ssl, serverConfiguration, mongoConnection); // Service connection

            int sd = SSL_get_fd(ssl);       /* get socket connection */
            SSL_free(ssl);         /* release SSL state */
            close(sd);          /* close connection */
            close(client);          /* close connection */

            printf("Connection closed.\n");

            break;
        } else if (pid < 0) {
            // Fork error
            printf("PID: %d\n", pid);
            break;
        }
    }

    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
    return EXIT_SUCCESS;
}

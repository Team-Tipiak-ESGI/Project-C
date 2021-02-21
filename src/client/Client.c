// TODO : faire les dossiers nessaires pour la gestion des conffiles cote user : .tucs/ conf/ pubKey/ privKey/
// TODO : SSL - Generation de clefs
// TODO - sub SSL - Gestion des clefs
// TODO : Gestion des fichiers de facon dynamique
// TODO : interface ncurses
// TODO : login / password

#include <stdio.h>
#include <string.h> // bzero
#include <unistd.h> /* close */
#include <netdb.h> // gethostbyname

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "Connection.h"
#include "SendFile.h"


int main(int argc, char** argv) {
    printf("%d\n", argc);
    
    SSL_CTX *ctx;
    int server;
    SSL *ssl;

    SSL_library_init();

    ctx = initCTX();
    server = openConnection("127.0.0.1", 8080);
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if (SSL_connect(ssl) < 0) {   /* perform the connection */
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

        /*printf("Commands:\n"
               "q: Quit\n"
               "u [username]: Set username\n"
               "p [password]: Set password\n"
               "l: Login\n"
               "s: Signup\n"
               "h: Help (this)\n"
               "ls: List files\n"
               "dl [filename]: Download file\n"
               "up [filename]: Upload file\n"
               "dl [filename]: Delete file\n");*/

        char * input = malloc(1024);

        char * username;
        char * password;

        while (1) {
            fputs("Enter command: ", stdin);
            fgets(input, 1024, stdin);

            strtok(input, "\n"); // Remove line break

            char * tok = input;
            char * action = strtok(tok, " ");
            tok = NULL;
            char * value = strtok(tok, " ");
            tok = NULL;

            if (!strcmp(action, "q"))
                break;
            else if (!strcmp(action, "u"))
                username = strdup(value);
            else if (!strcmp(action, "p"))
                password = strdup(value);
            else if (!strcmp(action, "l"))
                login(ssl, username, password);
            else if (!strcmp(action, "s"))
                signup(ssl, username, password);
            else if (!strcmp(action, "h"))
                printf("Commands:\n"
                       "q: Quit\n"
                       "u [username]: Set username\n"
                       "p [password]: Set password\n"
                       "l: Login\n"
                       "s: Signup\n"
                       "h: Help (this)\n"
                       "ls: List files\n"
                       "dl [filename]: Download file\n"
                       "up [filename]: Upload file\n"
                       "rm [filename]: Remove file\n");
            else if (!strcmp(action, "up"))
                sendFileToSocket(ssl, value);
            else if (!strcmp(action, "dl"))
                downloadFile(ssl, value, "/home/erwan/downloaded");
            else if (!strcmp(action, "rm"))
                deleteFile(ssl, value);
            else if (!strcmp(action, "ls"))
                listFiles(ssl);
        }

        //signup(ssl, "bonjour", "test");

        /*login(ssl, argv[1], argv[2]);

        sendFileToSocket(ssl, argv[3]);

        char ** files = listFiles(ssl);
        printf("File: %s\n", files[0]);

        downloadFile(ssl, files[0], "/home/erwan/downloaded");
        deleteFile(ssl, files[0]);*/

        SSL_free(ssl);        /* release connection state */
    }

    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */

    return EXIT_SUCCESS;
}

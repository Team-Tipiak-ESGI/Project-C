#include <stdio.h>
#include <string.h> // bzero

#include <openssl/bio.h>
#include <openssl/err.h>

#include "../shared/ChunkSize.h"

#include "../shared/PacketTypes.h"
#include "Connection.h"
#include "SendFile.h"
#include "window.h"
#include "../shared/Configuration.h"

void cli(SSL*ssl) {
    char * input = malloc(1024);

    char * username;
    char * password;

    fputs("Type (h) for help.\n", stdout);

    while (1) {
        fputs("> ", stdout);
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
                   "h: Help (this)\n"
                   "u [username]: Set username\n"
                   "p [password]: Set password\n"
                   "l: Login\n"
                   "s: Signup\n"
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
        else
            printf("Unknown command, try (h) for help.\n");
    }

    free(input);
}
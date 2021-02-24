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
#include "window.h"
#include "cli.h"
#include "../shared/Configuration.h"

#include <pwd.h>
#include <sys/stat.h>


int main(int argc, char** argv) {
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

    SSL_CTX *ctx;
    int server;
    SSL *ssl;

    SSL_library_init();

    ctx = initCTX();
    server = openConnection(Item__getByKey(config, "address")->value, atoi(Item__getByKey(config, "port")->value));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */

    if (SSL_connect(ssl) < 0) {   /* perform the connection */
        ERR_print_errors_fp(stderr);
    } else {
        printf("Connected with %s encryption.\n", SSL_get_cipher(ssl));

        if (argc > 1 && !strcmp(argv[1], "gui")) {
            window_login(ssl);
            window_items(ssl);
        } else {
            cli(ssl);
        }

        SSL_free(ssl);        /* release connection state */
    }

    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */

    return EXIT_SUCCESS;
}

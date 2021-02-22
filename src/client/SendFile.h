int sendFileToSocket(SSL *ssl, const char* filename);
int login(SSL *ssl, const char* username, const char* password);
int signup(SSL *ssl, const char* username, const char* password);
char ** listFiles(SSL *ssl);
int downloadFile(SSL *ssl, char * fileName, char * destination);
int deleteFile(SSL *ssl, char* fileName);
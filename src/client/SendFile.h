int sendFileToSocket(SSL *ssl, const char* filename);
void login(SSL *ssl, const char* username, const char* password);
int signup(SSL *ssl, const char* username, const char* password);
char ** listFiles(SSL *ssl);
void downloadFile(SSL *ssl, char * fileName, char * destination);
void deleteFile(SSL *ssl, char* fileName);
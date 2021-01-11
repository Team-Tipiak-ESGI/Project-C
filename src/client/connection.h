void loadCertificates(SSL_CTX *ctx, char *CertFile, char *KeyFile);

int openConnection(const char *hostname, int port);

SSL_CTX *initCTX(void);

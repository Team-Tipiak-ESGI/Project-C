unsigned char verifyUser(char* username, char* password, MongoConnection* mongoConnection);
char* hexHash(char * input);
char * getUserDir(const char* username, const char* password);
char* createFile(const char* username, const char* password, const char* fileName, const char* rootDir);
void writeChunk(const char* originalFilePath, const char* content, int chunkNumber);
void writePacket(SSL * ssl, char packet, char * content);

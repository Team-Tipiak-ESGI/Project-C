#include <mongoc.h>

MongoConnection* MongoConnection__init(char * uri);
void MongoConnection__close(MongoConnection* mongoConnection);
int MongoConnection__createUser(MongoConnection* mongoConnection, char* username, char* password);
int MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password);
void MongoConnection__addFile(MongoConnection* mongoConnection, char* username, char* password, char* fileName, char* filePath);
void MongoConnection__listFile(MongoConnection* mongoConnection, char* username, char* password);
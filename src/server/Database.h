#include <mongoc.h>

MongoConnection* MongoConnection__init(char * uri);
void MongoConnection__close(MongoConnection* mongoConnection);
void MongoConnection__createUser(MongoConnection* mongoConnection, char* username, char* password);
int MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password);